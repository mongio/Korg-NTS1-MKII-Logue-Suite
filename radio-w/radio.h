#pragma once
/*
 *  File: radio.h  (RADIO-W v0.1: variante di RADIO con il fischio regolabile)
 *  Radio a valvole anni '60: sintonia, bande FM/AM/SW/POCKET, fischio di battimento,
 *  banda laterale stonata (SSB), fading di propagazione, stazione fantasma,
 *  statico, ronzio a 50 Hz, deriva e ricerca automatica della sintonia.
 *  Nessuna dipendenza da libm e da tabelle del firmware.
 */
#include <stdint.h>

class Radio {
 public:
  enum { P_TUNE = 0, P_AGE, P_BAND, P_FADING, P_DRIFT, P_STATIC, P_GHOST, P_WHISTLE, P_SEEK, P_MIX, P_NUM };
  enum { B_FM = 0, B_AM, B_SW, B_POCKET };
  static constexpr float kSR = 48000.f;
  static constexpr float kMain = 0.5f, kGhostA = 0.17f, kGhostB = 0.84f;

  void init(float *ghost_buf, uint32_t size_pow2) {
    gbuf_ = ghost_buf; gsize_ = size_pow2; gmask_ = size_pow2 - 1; gw_ = 0;
    tune_ = 0.5f; age_ = 0.4f; band_ = B_AM; fading_ = 0.3f; drift_ = 0.2f; static_ = 0.4f;
    ghost_ = 0.3f; whistle_ = 0.4f; seek_ = 0.f; mix_ = 1.f;
    rng_ = 0x52414449u;
    dial_ = 0.5f; seek_pos_ = 0.5f; seek_target_ = 0.5f; seek_dwell_ = 0.f;
    clear();
  }

  void clear() {
    if (gbuf_) for (uint32_t i = 0; i < gsize_; ++i) gbuf_[i] = 0.f;
    for (int i = 0; i < 256; ++i) comb_[i] = 0.f;
    comb_w_ = 0;
    hpm_.reset(); lpm_.reset(); hpg_.reset(); lpg_.reset(); spk_.reset(); nlp_.reset();
    for (int i = 0; i < 8; ++i) { hx1_[i] = hx2_[i] = hy1_[i] = hy2_[i] = 0.f; }
    hb_delay_ = 0.f;
    dcx_ = dcy_ = 0.f; r1_ = r2_ = r3_ = 0.f; rw_ = 0.f; crack_ = 0.f; pink_ = 0.f;
    sr1_ = sr2_ = sr3_ = SlowRand{0.f, 0.f, 0.f, 0.f, 0};
    // oscillatori in quadratura (ricorsivi)
    wc_ = 1.f; ws_ = 0.f; sc_ = 1.f; ss_ = 0.f; hc_ = 1.f; hs_ = 0.f;
  }

  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_TUNE:   tune_ = v * (1.f / 1023.f); break;
      case P_AGE:    age_ = v * (1.f / 1023.f); break;
      case P_BAND:   band_ = (uint8_t)v; break;
      case P_FADING: fading_ = v * 0.01f; break;
      case P_DRIFT:  drift_ = v * 0.01f; break;
      case P_STATIC: static_ = v * 0.01f; break;
      case P_GHOST:  ghost_ = v * 0.01f; break;
      case P_WHISTLE: whistle_ = v * 0.01f; break;
      case P_SEEK:   seek_ = v * 0.01f; break;
      case P_MIX:    mix_ = v * 0.01f; break;
      default: break;
    }
  }

  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *bands[] = {"FM", "AM", "SW", "POCKET"};
    if (id == P_BAND && v >= 0 && v < 4) return bands[v];
    return nullptr;
  }

  void process(const float *in, float *out, uint32_t frames) {
    // --- impostazioni per banda (calcolate una volta per buffer) ---
    static const float hp_hz[4] = {50.f, 150.f, 300.f, 380.f};
    static const float lp_hz[4] = {11000.f, 4500.f, 3000.f, 3400.f};
    static const float width[4] = {0.05f, 0.07f, 0.04f, 0.075f};
    static const float fade_k[4] = {0.3f, 0.6f, 0.95f, 0.5f};
    static const float comb_k[4] = {0.15f, 0.35f, 0.8f, 0.3f};
    static const float makeup[4] = {1.39f, 1.48f, 1.58f, 1.21f};
    const uint8_t b = band_ < 4 ? band_ : B_AM;

    const float lp = lp_hz[b] * (1.f - 0.6f * age_);
    const float hp = hp_hz[b] * (1.f + 1.5f * age_);
    hpm_.set(hp, 0.7f); lpm_.set(lp, b == B_POCKET ? 1.6f : 0.75f);
    hpg_.set(hp * 1.6f, 0.7f); lpg_.set(lp * 0.6f, 0.9f);
    spk_.set(1200.f, 2.5f);
    nlp_.set(lp, 0.7f);
    const float drv = 1.f + age_ * 4.f + (b == B_POCKET ? 1.5f : 0.f);
    const float drv_mk = (1.f + 0.3f * (drv - 1.f)) / drv;   // guadagno ~1 a basso livello, cresce poco col drive
    const float w = width[b];

    for (uint32_t i = 0; i < frames; ++i) {
      const float inL = in[2 * i], inR = in[2 * i + 1];
      const float x = (inL + inR) * 0.5f;

      // --- moti lenti casuali: deriva, fading, ricerca ---
      r1_ = slow(sr1_, 1.6f);                // fading (~0.3 Hz)
      r2_ = slow(sr2_, 0.7f);                // notch mobile / fantasma
      r3_ = slow(sr3_, 6.f);                 // deriva della sintonia
      updateSeek();
      float target = (seek_ > 0.f) ? seek_pos_ + (tune_ - 0.5f) * 0.1f : tune_;
      target += r3_ * drift_ * 0.12f;
      dial_ += (target - dial_) * 0.0008f;

      // --- quanto e' sintonizzata ciascuna stazione ---
      // nitidezza delle stazioni: la manopola si muove lentamente, si ricalcola ogni 16 campioni
      if ((i & 15u) == 0u) {
        const float dga = dial_ - kGhostA, dgb = dial_ - kGhostB;
        dm_ = dial_ - kMain;
        cm_ = clarity(dm_, w); cg_ = clarity(dga, w * 0.8f);
        const float cgb = clarity(dgb, w * 0.8f); if (cgb > cg_) cg_ = cgb;
        if (b == B_FM) { cm_ = cm_ * cm_ * (3.f - 2.f * cm_); cg_ = cg_ * cg_; }   // FM: aggancio piu' netto
        dn_ = absf(dm_); if (absf(dga) < dn_) dn_ = absf(dga); if (absf(dgb) < dn_) dn_ = absf(dgb);
      }
      const float dm = dm_, cm = cm_, cg = cg_, dn = dn_;
      const float cany = cm > cg ? cm : cg;

      // --- stazione principale ---
      float s = hpm_.hp(x);
      s = lpm_.lp(s);
      if (b == B_POCKET) s += spk_.bp(s) * 0.8f;
      if (b == B_SW) {                        // banda laterale stonata ("paperella")
        float sh = dm * 2600.f; sh = sh > 450.f ? 450.f : (sh < -450.f ? -450.f : sh);
        s = freqShift(s, sh);
      }
      if (b == B_FM && cm < 0.6f) s = hardish(s * (1.f + (0.6f - cm) * 6.f)) * 0.6f;   // distorsione multipath

      // fading di propagazione: guadagno + notch mobile (comb)
      const float fg = 1.f - fading_ * fade_k[b] * (0.5f + 0.5f * r1_);
      s *= clamp(fg, 0.05f, 1.f);
      s = combFade(s, fading_ * comb_k[b]);
      s *= cm;

      // --- stazione fantasma: l'ingresso ritardato, filtrato piu' stretto ---
      float g = 0.f;
      if (gbuf_) {
        gbuf_[gw_] = x; gw_ = (gw_ + 1) & gmask_;
        if (ghost_ > 0.f) {
          const float dsec = 0.42f + r2_ * 0.08f;
          float dly = dsec * kSR; if (dly > gsize_ - 4) dly = (float)(gsize_ - 4);
          const int32_t di = (int32_t)dly; const float df = dly - di;
          const float a = gbuf_[(gw_ - 1 - di) & gmask_], c = gbuf_[(gw_ - 2 - di) & gmask_];
          g = a + (c - a) * df;
          g = lpg_.lp(hpg_.hp(g));
          const float bleed = (1.f - cm) * 0.25f;          // tra le stazioni si sente in sottofondo
          g *= ghost_ * (cg + bleed * (1.f - cg)) * (0.6f + 0.4f * r2_);
        }
      }

      // --- fischio di battimento (AM/SW/POCKET) ---
      float wh = 0.f;
      if (b != B_FM) {
        const float fw = 60.f + dn * 9000.f;
        rotate(wc_, ws_, fw);
        const float near = cany * (1.f - cany) * 4.f;     // massimo a meta' sintonia
        wh = ws_ * near * 0.06f * whistle_ * 2.f;
      }

      // --- statico: fruscio + crepitii ---
      const float wn = randf();
      pink_ = pink_ * 0.97f + wn * 0.03f;
      float noise = (b == B_FM) ? (wn - pink_) * 0.8f : (pink_ * 4.f + wn * 0.25f);
      noise = nlp_.lp(noise);
      const float nlev = static_ * (0.03f + 0.5f * (1.f - cany)) + age_ * 0.01f;
      if (rand01() < (static_ * (1.f - cany) * 0.002f + age_ * 0.00015f)) crack_ = randf() * 0.5f;
      crack_ *= 0.7f;
      noise = noise * nlev * 0.35f + crack_ * (static_ + age_) * 0.5f;

      // --- ronzio di rete a 50 Hz ---
      rotate(hc_, hs_, 50.f);
      const float hm = age_ * 0.012f;                  // ronzio automatico: solo con l'eta' della radio
      const float h2 = 2.f * hs_ * hc_, h3 = hs_ * (3.f - 4.f * hs_ * hs_);   // 100 e 150 Hz
      const float humv = (hs_ + 0.5f * h2 + 0.3f * h3) * hm;

      // --- valvola finale: saturazione asimmetrica + blocco DC ---
      float y = (s + g + wh + noise + humv) * makeup[b];
      float z = y * drv; z = z + 0.18f * z * z * (z > 0.f ? 1.f : -0.4f);
      y = z / (1.f + absf(z)) * drv_mk;
      const float dy = y - dcx_ + 0.9975f * dcy_; dcx_ = y; dcy_ = dy;
      y = dy;
      // limitatore morbido
      const float ay = absf(y);
      if (ay > 0.8f) { const float e = (ay - 0.8f) * 5.f; const float lim = 0.8f + 0.2f * e / (1.f + e); y = y < 0.f ? -lim : lim; }

      out[2 * i]     = inL * (1.f - mix_) + y * mix_;
      out[2 * i + 1] = inR * (1.f - mix_) + y * mix_;
    }
  }

 private:
  // ---------- filtro a variabile di stato (TPT) ----------
  struct Svf {
    float g, k, a1, a2, a3, ic1, ic2;
    void reset() { ic1 = ic2 = 0.f; set(1000.f, 0.7f); }
    void set(float fc, float q) {
      if (fc > 18000.f) fc = 18000.f; if (fc < 20.f) fc = 20.f;
      const float x = 3.14159265f * fc / kSR;           // tan(x) per serie
      const float x2 = x * x;
      g = x * (1.f + x2 * (0.3333333f + x2 * (0.1333333f + x2 * 0.0539683f)));
      k = 1.f / q;
      a1 = 1.f / (1.f + g * (g + k)); a2 = g * a1; a3 = g * a2;
    }
    inline void tick(float v0, float &lo, float &bd) {
      const float v3 = v0 - ic2, v1 = a1 * ic1 + a2 * v3, v2 = ic2 + a2 * ic1 + a3 * v3;
      ic1 = 2.f * v1 - ic1; ic2 = 2.f * v2 - ic2; lo = v2; bd = v1;
    }
    inline float lp(float v) { float l, b; tick(v, l, b); return l; }
    inline float bp(float v) { float l, b; tick(v, l, b); return b; }
    inline float hp(float v) { float l, b; tick(v, l, b); return v - k * b - l; }
  };

  // ---------- utilita' ----------
  static inline float absf(float x) { return x < 0.f ? -x : x; }
  static inline float clamp(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }
  static inline float clarity(float d, float w) { const float u = d / w; return 1.f / (1.f + u * u); }
  static inline float hardish(float x) { return x / (1.f + absf(x) * 0.8f); }
  inline uint32_t nextRand() { uint32_t r = rng_; r ^= r << 13; r ^= r >> 17; r ^= r << 5; rng_ = r; return r; }
  inline float rand01() { return (nextRand() >> 8) * (1.f / 16777216.f); }
  inline float randf() { return rand01() * 2.f - 1.f; }

  // valore casuale lento: nuovo obiettivo ogni ~sec secondi, interpolato e smussato (-1..1)
  struct SlowRand { float v, t, step, sm; int32_t n; };
  float slow(SlowRand &r, float sec) {
    if (--r.n <= 0) {
      r.n = (int32_t)(kSR * sec * (0.5f + rand01()));
      r.t = randf();
      r.step = (r.t - r.v) / (float)r.n;
    }
    r.v += r.step;
    r.sm += (r.v - r.sm) * 0.0005f;
    return r.sm;
  }

  // oscillatore in quadratura ricorsivo, con rinormalizzazione
  static inline void rotate(float &c, float &s, float hz) {
    const float th = 6.2831853f * hz / kSR;
    const float k1 = th, k2 = 1.f - th * th * 0.5f;
    const float nc = c * k2 - s * k1, ns = s * k2 + c * k1;
    const float n = 1.5f - 0.5f * (nc * nc + ns * ns);
    c = nc * n; s = ns * n;
  }

  // spostatore di frequenza: Hilbert IIR (due catene di allpass) + modulazione in quadratura
  float freqShift(float x, float hz) {
    static const float ca[4] = {0.6923878f, 0.9360654322959f, 0.9882295226860f, 0.9987488452737f};
    static const float cb[4] = {0.4021921162426f, 0.8561710882420f, 0.9722909545651f, 0.9952884791278f};
    float a = x, bb = x;
    for (int i = 0; i < 4; ++i) {
      const float c2 = ca[i] * ca[i];
      const float y = c2 * (a + hy2_[i]) - hx2_[i];
      hx2_[i] = hx1_[i]; hx1_[i] = a; hy2_[i] = hy1_[i]; hy1_[i] = y; a = y;
    }
    for (int i = 0; i < 4; ++i) {
      const int j = i + 4; const float c2 = cb[i] * cb[i];
      const float y = c2 * (bb + hy2_[j]) - hx2_[j];
      hx2_[j] = hx1_[j]; hx1_[j] = bb; hy2_[j] = hy1_[j]; hy1_[j] = y; bb = y;
    }
    const float bq = hb_delay_; hb_delay_ = bb;          // la catena B va ritardata di 1 campione
    rotate(sc_, ss_, hz < 0.f ? -hz : hz);
    return hz >= 0.f ? (a * sc_ + bq * ss_) : (a * sc_ - bq * ss_);
  }

  // notch mobile da fading selettivo: comb con ritardo che scorre lentamente
  float combFade(float x, float depth) {
    comb_[comb_w_] = x;
    if (depth <= 0.001f) { comb_w_ = (comb_w_ + 1) & 255; return x; }
    const float d = 24.f + (0.5f + 0.5f * r2_) * 200.f;
    const int32_t di = (int32_t)d; const float df = d - di;
    const float a = comb_[(comb_w_ - di) & 255], c = comb_[(comb_w_ - di - 1) & 255];
    comb_w_ = (comb_w_ + 1) & 255;
    return (x + (a + (c - a) * df) * depth) / (1.f + depth * 0.5f);
  }

  // ricerca automatica: scorre la sintonia, si ferma un po' su stazioni o rumore
  void updateSeek() {
    if (seek_ <= 0.f) { seek_pos_ = tune_; return; }
    const float speed = (0.03f + seek_ * seek_ * 0.5f) / kSR;
    const float d = seek_target_ - seek_pos_;
    if (absf(d) > speed) { seek_pos_ += d > 0.f ? speed : -speed; return; }
    seek_pos_ = seek_target_;
    seek_dwell_ -= 1.f;
    if (seek_dwell_ > 0.f) return;
    seek_dwell_ = kSR * (0.6f + 3.f * rand01()) / (0.3f + seek_);
    const uint32_t r = nextRand() % 6;
    seek_target_ = r == 0 ? kGhostA : (r == 1 ? kGhostB : (r <= 3 ? kMain : rand01()));
    seek_target_ += randf() * 0.01f;
  }

  // buffer fantasma (SDRAM)
  float *gbuf_ = nullptr; uint32_t gsize_ = 0, gmask_ = 0, gw_ = 0;
  // parametri
  float tune_, age_, fading_, drift_, static_, ghost_, whistle_, seek_, mix_;
  uint8_t band_;
  // stato
  uint32_t rng_;
  float dial_, seek_pos_, seek_target_, seek_dwell_;
  Svf hpm_, lpm_, hpg_, lpg_, spk_, nlp_;
  float hx1_[8], hx2_[8], hy1_[8], hy2_[8], hb_delay_;
  float comb_[256]; uint32_t comb_w_;
  float dcx_, dcy_, r1_, r2_, r3_, rw_, crack_, pink_;
  float wc_, ws_, sc_, ss_, hc_, hs_;
  SlowRand sr1_, sr2_, sr3_;
  float dm_ = 0.f, cm_ = 1.f, cg_ = 0.f, dn_ = 0.f;
};
