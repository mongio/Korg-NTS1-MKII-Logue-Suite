#pragma once
/*
 *  File: ricordo.h  (RICORDO v0.2: FREEZE a livello costante, saturazione senza perdita di livello)
 *  Riverbero FDN a 8 linee (Hadamard) con una catena di invecchiamento DENTRO l'anello:
 *  a ogni ricircolo la coda perde banda, si satura, perde bit, prende soffio, wow e cadute.
 *  L'attacco resta limpido, la coda diventa un vecchio supporto (nastro, vinile, radio, cera).
 *  Nessuna dipendenza da libm.
 */
#include <stdint.h>

static const uint32_t kApLen[8] = {142, 107, 379, 277, 149, 113, 389, 283};   // diffusori d'ingresso

class Ricordo {
 public:
  enum { P_DECAY = 0, P_AGE, P_MIX, P_ERA, P_SIZE, P_TONE, P_WOW, P_GRAIN, P_HISS, P_SINK, P_FREEZE, P_NUM };
  enum { E_NASTRO = 0, E_VINILE, E_RADIO, E_CERA };
  enum { NL = 8, LSIZE = 8192, LMASK = LSIZE - 1 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kBufFloats = NL * LSIZE + 8 * 512;   // linee + diffusori, tutto in SDRAM

  void init(float *buf) {
    for (int i = 0; i < NL; ++i) { line_[i] = buf + i * LSIZE; }
    { float *p = apmem_; for (int i = 0; i < 8; ++i) { ap_[i] = p; p += kApLen[i]; } }   // diffusori nella RAM interna (piu' veloce della SDRAM)
    decay_ = 0.6f; age_ = 0.45f; mix_ = 0.f; era_ = E_NASTRO; size_ = 0.6f; tone_ = 0.7f;
    wow_ = 0.2f; grain_ = 0.f; hiss_ = 0.2f; sink_ = 0.f; freeze_ = false;
    rng_ = 0x5249434Fu;
    clear();
  }

  void clear() {
    for (int i = 0; i < NL; ++i) {
      for (uint32_t k = 0; k < LSIZE; ++k) line_[i][k] = 0.f;
      lp_[i] = lp2_[i] = hp_[i] = hpx_[i] = 0.f; hold_[i] = 0.f; drop_[i] = 1.f; drop_t_[i] = 1.f;
      mph_[i] = 0.137f * i; 
    }
    for (uint32_t k = 0; k < kApTotal; ++k) apmem_[k] = 0.f;
    for (int i = 0; i < 8; ++i) ap_i_[i] = 0;
    for (int i = 0; i < 16; ++i) hx1_[i] = hx2_[i] = hy1_[i] = hy2_[i] = 0.f;
    hbd_[0] = hbd_[1] = 0.f; shc_ = 1.f; shs_ = 0.f;
    w_ = 0; env_ = 0.f; hold_cnt_ = 0; crack_ = 0.f; rumble_ = 0.f;
  }

  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_DECAY:  decay_ = v * (1.f / 1023.f); break;
      case P_AGE:    age_ = v * (1.f / 1023.f); break;
      case P_MIX:    mix_ = v * 0.001f; break;
      case P_ERA:    era_ = (uint8_t)v; break;
      case P_SIZE:   size_ = v * 0.01f; break;
      case P_TONE:   tone_ = v * 0.01f; break;
      case P_WOW:    wow_ = v * 0.01f; break;
      case P_GRAIN:  grain_ = v * 0.01f; break;
      case P_HISS:   hiss_ = v * 0.01f; break;
      case P_SINK:   sink_ = v * 0.01f; break;
      case P_FREEZE: { const bool f = (v != 0); if (f && !freeze_) { fz_wait_ = (int32_t)(kSR * 0.05f); fz_gain_ = 1.f; } freeze_ = f; } break;
      default: break;
    }
  }

  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *eras[] = {"NASTRO", "VINILE", "RADIO", "CERA"};
    if (id == P_ERA && v >= 0 && v < 4) return eras[v];
    return nullptr;
  }

  void process(const float *in, float *out, uint32_t frames) {
    // ---- caratteri delle ere: [lp invecchiato Hz, hp invecchiato Hz, saturazione, bit persi, soffio, wow ms, crepitii, rate wow Hz]
    static const float E_LP[4]   = {4200.f, 3400.f, 2900.f, 2500.f};
    static const float E_HP[4]   = {140.f, 90.f, 420.f, 480.f};
    static const float E_SAT[4]  = {1.6f, 0.8f, 1.0f, 0.6f};
    static const float E_BITS[4] = {2.f, 3.f, 4.f, 6.f};
    static const float E_HISS[4] = {0.005f, 0.002f, 0.006f, 0.011f};
    static const float E_WOW[4]  = {1.2f, 1.6f, 0.4f, 2.6f};
    static const float E_CRK[4]  = {0.f, 0.0006f, 0.00015f, 0.0004f};
    static const float E_RATE[4] = {0.6f, 0.555f, 0.25f, 1.3f};      // vinile: 33 1/3 giri = 0,555 Hz
    static const float base_ms[NL] = {37.1f, 41.3f, 47.9f, 53.3f, 59.9f, 67.1f, 73.7f, 79.3f};
    const uint8_t e = era_ < 4 ? era_ : 0;

    // ---- lunghezze, guadagni (RT60), filtri dell'anello: per buffer
    const float scale = 0.35f + size_ * 1.25f;                       // 0.35x .. 1.6x
    const float rt_set = 0.5f * pow2(decay_ * 5.907f);               // 0.5 .. 30 s
    const float rt = rt_set * (1.08f + 0.16f * decay_);              // calibrazione: perdite di interpolazione/filtri
    float len[NL], g[NL];
    for (int i = 0; i < NL; ++i) {
      len[i] = base_ms[i] * scale * 48.f;
      g[i] = freeze_ ? fz_gain_ : pow2neg(9.9658f * len[i] / (rt * kSR));  // 10^(-3 L / (RT fs))
    }
    // filtri: puliti (TONE) -> invecchiati (ERA) secondo AGE; in FREEZE il solo TONE non smorza
    const float lp_clean = 2500.f * pow2(tone_ * 2.85f);             // 2.5 .. 18 kHz
    const float lp_hz = lerp(lp_clean, E_LP[e] < lp_clean ? E_LP[e] : lp_clean, age_);
    const float hp_hz = lerp(freeze_ ? 6.f : 20.f, E_HP[e], age_);
    const float c_lp = ((freeze_ || tone_ > 0.99f) && age_ < 0.01f) ? 1.f : onePole(lp_hz);
    const float c_hp = onePole(hp_hz);
    const float sat = 1.f + age_ * E_SAT[e] * 2.f;
    const float bits = 16.f - grain_ * 10.f - age_ * E_BITS[e];
    const float qstep = bits < 15.5f ? pow2neg(bits - 1.f) : 0.f;
    const int32_t hold_n = 1 + (int32_t)(grain_ * grain_ * 5.f);     // riduzione di frequenza di campionamento
    const float hiss_amt = hiss_ * 0.012f + age_ * E_HISS[e];
    const float wow_smp = (0.12f + wow_ * 2.5f + age_ * E_WOW[e]) * 48.f;
    const float wow_rate = E_RATE[e];
    const float crk = age_ * E_CRK[e];
    const float sink_hz = -sink_ * 20.f;
    const float in_g = freeze_ ? 0.f : 1.f;

    const float m = (mix_ + 1.f) * 0.5f;
    const float dry_g = m < 0.5f ? 1.f : 2.f * (1.f - m);
    const float wet_g = m > 0.5f ? 1.f : 2.f * m;

    // wow a blocchi (oscillazione lentissima): ritardo di ogni linea interpolato linearmente nel blocco
    float d0[NL], dd[NL];
    for (int i = 0; i < NL; ++i) {
      const float inc = wow_rate * (0.85f + 0.06f * i) / kSR * (float)frames;
      const float a = len[i] + wow_smp * (1.f + sinp(mph_[i]));
      mph_[i] += inc; if (mph_[i] >= 1.f) mph_[i] -= 1.f;
      const float b = len[i] + wow_smp * (1.f + sinp(mph_[i]));
      d0[i] = a; dd[i] = (b - a) / (float)frames;
    }
    const float inv_q = qstep > 0.f ? 1.f / qstep : 0.f;
    // cadute di nastro: estrazione una volta per blocco (stessa probabilita' media)
    const float p_drop = age_ * 0.000004f * (float)frames;
    // silenzio: se ingresso e coda sono spenti da 0,5 s, niente calcolo (solo il dry)
    {
      float pk = 0.f; for (uint32_t k = 0; k < 2 * frames; ++k) { const float a = absf(in[k]); if (a > pk) pk = a; }
      if (pk > 1e-5f || freeze_ || env_ > 2e-6f) idle_ = 0; else if (idle_ < 0x7FFFFFFF) idle_ += (int32_t)frames;
      if (idle_ > (int32_t)(kSR * 0.5f)) { for (uint32_t k = 0; k < 2 * frames; ++k) out[k] = in[k] * dry_g; return; }
    }
    for (uint32_t n = 0; n < frames; ++n) {
      const float inL = in[2 * n], inR = in[2 * n + 1];

      // ---- diffusione d'ingresso (4 allpass per lato)
      float dl = inL * in_g, dr = inR * in_g;
      dl = allpass(0, dl, 142, 0.7f); dl = allpass(1, dl, 107, 0.7f); dl = allpass(2, dl, 379, 0.7f); dl = allpass(3, dl, 277, 0.7f);
      dr = allpass(4, dr, 149, 0.7f); dr = allpass(5, dr, 113, 0.7f); dr = allpass(6, dr, 389, 0.7f); dr = allpass(7, dr, 283, 0.7f);

      // ---- lettura delle linee, con wow (modulazione lenta, fase diversa per linea)
      float x[NL];
      for (int i = 0; i < NL; ++i) {
        const float d = d0[i] + dd[i] * (float)n;
        const float rp = (float)w_ - d;
        int32_t ip = (int32_t)rp; if (rp < 0.f && (float)ip != rp) --ip;
        const float f = rp - (float)ip;
        const float a = line_[i][(uint32_t)ip & LMASK], b = line_[i][(uint32_t)(ip + 1) & LMASK];
        x[i] = a + (b - a) * f;
      }

      // ---- uscita (prima della matrice), stereo con segni diversi
      float oL = (x[0] - x[1] + x[2] - x[3] + x[4] - x[5] + x[6] - x[7]) * 0.35f;
      float oR = (x[0] + x[1] - x[2] - x[3] + x[4] + x[5] - x[6] - x[7]) * 0.35f;

      // ---- matrice di Hadamard 8x8 (butterfly), normalizzata
      hadamard(x);

      // ---- inviluppo della coda (per il soffio proporzionale)
      const float lvl = absf(x[0]) + absf(x[3]) + absf(x[5]);
      env_ += (lvl - env_) * 0.0005f;
      // FREEZE: memorizza il livello della coda e lo mantiene costante (l'invecchiamento cambia il colore, non il volume)
      if (freeze_) {
        if (fz_wait_ > 0) { if (--fz_wait_ == 0) fz_ref_ = env_ > 1e-5f ? env_ : 1e-5f; }
        else if (env_ > 1e-6f) {
          float err = fz_ref_ / env_ - 1.f; err = err > 1.f ? 1.f : (err < -0.5f ? -0.5f : err);
          fz_gain_ += err * 0.0001f;
          fz_gain_ = fz_gain_ < 0.85f ? 0.85f : (fz_gain_ > 1.3f ? 1.3f : fz_gain_);
        }
      }

      // ---- crepitii (vinile/cera): impulsi che poi riverberano
      if (crk > 0.f && rand01() < crk * (env_ > 0.002f ? 1.f : 0.f)) { const float ev = env_ * 6.f; crack_ = randf() * 0.25f * age_ * (ev > 1.f ? 1.f : ev); }   // crepitii proporzionali alla coda
      const float ck = crack_; crack_ *= 0.55f;
      if (e == E_VINILE) { rumble_ += (randf() - rumble_) * 0.0008f; }

      // ---- SINK: spostatore di frequenza su due linee (la coda scende d'intonazione)
      if (sink_ > 0.001f) {
        rotate(shc_, shs_, sink_hz);          // frequenza negativa: la coda scende
        x[0] = freqShift(x[0], 0);
        x[4] = freqShift(x[4], 1);
      }

      // ---- catena di invecchiamento per linea, poi scrittura
      uint32_t nz = nextRand();                         // un'estrazione per campione, poi rumore per linea
      const bool do_hold = (hold_n > 1) && (++hold_cnt_ >= hold_n);
      if (do_hold) hold_cnt_ = 0;
      for (int i = 0; i < NL; ++i) {
        float y = x[i] * g[i];
        // saturazione da supporto (guadagno unitario a basso livello)
        if (sat > 1.01f) { const float k = (sat - 1.f) * 0.12f; const float yy = y > 1.f ? 1.f : (y < -1.f ? -1.f : y); y = y - k * yy * yy * yy; }   // calore, quasi senza perdita di livello
        // grana: meno bit (con dither) e campioni tenuti
        nz = nz * 1664525u + 1013904223u;
        const float rn = (float)(int32_t)nz * (1.f / 2147483648.f);   // -1..1
        if (qstep > 0.f) { const float qd = (y + rn * qstep * 0.5f) * inv_q; y = (float)(int32_t)(qd + (qd >= 0.f ? 0.5f : -0.5f)) * qstep; }
        if (hold_n > 1) { if (do_hold) hold_[i] = y; else y = hold_[i]; }
        // soffio proporzionale alla coda, crepitii e rombo: entrano PRIMA dei filtri dell'epoca
        if (hiss_amt > 0.f) y += rn * hiss_amt * (env_ * 4.f + 0.0005f);
        y += ck;
        if (e == E_VINILE && i < 2) y += rumble_ * 0.02f * age_;
        // banda che si stringe (anche il rumore appena aggiunto)
        if (c_lp < 1.f) {
          lp_[i] += (y - lp_[i]) * c_lp; y = lp_[i];
          if (age_ > 0.01f) { lp2_[i] += (y - lp2_[i]) * c_lp; y = lp2_[i]; }   // 2 poli quando invecchia
        }
        hp_[i] += (y - hp_[i]) * c_hp; y = y - hp_[i];
        // cadute di nastro
        if (age_ > 0.05f) {
          if (n == 0 && rand01() < p_drop) drop_t_[i] = 0.4f + 0.4f * rand01();
          drop_[i] += (drop_t_[i] - drop_[i]) * 0.003f; drop_t_[i] += (1.f - drop_t_[i]) * 0.0003f;
          y *= drop_[i];
        }
        // iniezione dell'ingresso diffuso (L sulle pari, R sulle dispari)
        y += ((i & 1) ? dr : dl) * 0.6f * (((i >> 1) & 1) ? -1.f : 1.f);
        // sicurezza: limitatore trasparente sotto 0.6, morbido sopra
        { const float a = absf(y); if (a > 0.6f) { const float ex = a - 0.6f; const float l = 0.6f + ex / (1.f + ex * 2.f); y = y < 0.f ? -l : l; } }
        line_[i][w_] = y;
      }
      w_ = (w_ + 1) & LMASK;

      out[2 * n]     = inL * dry_g + softclip(oL) * wet_g;
      out[2 * n + 1] = inR * dry_g + softclip(oR) * wet_g;
    }
  }

 private:
  // ---------- utilita' ----------
  static inline float absf(float x) { return x < 0.f ? -x : x; }
  static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
  static inline float pow2(float x) {          // 2^x per x >= 0
    const int32_t ip = (int32_t)x; const float f = x - ip;
    return (float)(1u << ip) * (1.f + f * (0.6931472f + f * (0.2402265f + f * 0.0555041f)));
  }
  static inline float pow2neg(float x) {       // 2^-x per x >= 0 (fino a ~30)
    if (x > 30.f) return 0.f;
    return 1.f / pow2(x);
  }
  static inline float onePole(float hz) { const float x = 6.2831853f * hz / kSR; return 1.f - pow2neg(x * 1.4426950f); }
  static inline float softclip(float x) { if (x > 1.5f) return 1.f; if (x < -1.5f) return -1.f; return x - x * x * x * (4.f / 27.f); }
  static inline float sinp(float ph) { float x = ph < 0.5f ? ph * 4.f - 1.f : 3.f - ph * 4.f; return x * (1.5f - 0.5f * x * x); }
  inline uint32_t nextRand() { uint32_t r = rng_; r ^= r << 13; r ^= r >> 17; r ^= r << 5; rng_ = r; return r; }
  inline float rand01() { return (nextRand() >> 8) * (1.f / 16777216.f); }
  inline float randf() { return rand01() * 2.f - 1.f; }

  static inline void hadamard(float *x) {
    for (int s = 1; s < NL; s <<= 1)
      for (int i = 0; i < NL; i += 2 * s)
        for (int j = i; j < i + s; ++j) { const float a = x[j], b = x[j + s]; x[j] = a + b; x[j + s] = a - b; }
    for (int i = 0; i < NL; ++i) x[i] *= 0.35355339f;   // 1/sqrt(8)
  }

  inline float allpass(int k, float x, uint32_t len, float g) {
    float *b = ap_[k]; uint32_t &i = ap_i_[k];
    const float d = b[i]; const float y = d - g * x; b[i] = x + g * y;
    if (++i >= len) i = 0;
    return y;
  }

  static inline void rotate(float &c, float &s, float hz) {
    const float th = 6.2831853f * hz / kSR;
    const float nc = c * (1.f - th * th * 0.5f) - s * th, ns = s * (1.f - th * th * 0.5f) + c * th;
    const float k = 1.5f - 0.5f * (nc * nc + ns * ns); c = nc * k; s = ns * k;
  }
  // spostatore di frequenza (Hilbert IIR a due catene), ch = 0/1 per due linee indipendenti
  float freqShift(float x, int ch) {
    static const float ca[4] = {0.6923878f, 0.9360654322959f, 0.9882295226860f, 0.9987488452737f};
    static const float cb[4] = {0.4021921162426f, 0.8561710882420f, 0.9722909545651f, 0.9952884791278f};
    float a = x, b = x; const int o = ch * 8;
    for (int i = 0; i < 4; ++i) { const int j = o + i; const float c2 = ca[i] * ca[i]; const float y = c2 * (a + hy2_[j]) - hx2_[j]; hx2_[j] = hx1_[j]; hx1_[j] = a; hy2_[j] = hy1_[j]; hy1_[j] = y; a = y; }
    for (int i = 0; i < 4; ++i) { const int j = o + 4 + i; const float c2 = cb[i] * cb[i]; const float y = c2 * (b + hy2_[j]) - hx2_[j]; hx2_[j] = hx1_[j]; hx1_[j] = b; hy2_[j] = hy1_[j]; hy1_[j] = y; b = y; }
    const float bq = hbd_[ch]; hbd_[ch] = b;
    return a * shc_ + bq * shs_;
  }

  float *line_[NL];
  float decay_, age_, mix_, size_, tone_, wow_, grain_, hiss_, sink_;
  float fz_gain_ = 1.f, fz_ref_ = 0.f; int32_t fz_wait_ = 0;
  uint8_t era_; bool freeze_;
  uint32_t rng_, w_;
  float lp_[NL], lp2_[NL], hp_[NL], hpx_[NL], hold_[NL], drop_[NL], drop_t_[NL], mph_[NL];
  float *ap_[8]; uint32_t ap_i_[8];
  static constexpr uint32_t kApTotal = 142 + 107 + 379 + 277 + 149 + 113 + 389 + 283;
  float apmem_[kApTotal];
  float hx1_[16], hx2_[16], hy1_[16], hy2_[16], hbd_[2], shc_, shs_;
  float env_, crack_, rumble_;
  int32_t hold_cnt_, idle_ = 0;
};
