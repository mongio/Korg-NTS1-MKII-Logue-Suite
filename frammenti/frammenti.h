#pragma once
/*
 *  File: frammenti.h  (FRAMMENTI v0.2: finestre dei grani ricorsive, compilazione veloce)
 *  Delay granulare / micro-loop ispirato ai pedali "texture".
 *  Buffer mono in SDRAM (ingresso L+R sommato), grani spazializzati in stereo,
 *  diffusione riverberante interna. Nessuna dipendenza da libm.
 */
#include <stdint.h>

static const uint32_t kApLen[8] = {142, 107, 379, 277, 149, 113, 389, 283};

class Frammenti {
 public:
  enum { P_TIME = 0, P_DEPTH, P_MIX, P_MODE, P_FB, P_SPACE, P_TONE, P_PITCH, P_SYNC, P_HOLD, P_SPREAD, P_NUM };
  enum { M_CLOUD = 0, M_LOOP, M_REVERSE, M_TAPS };
  enum { MAX_GRAINS = 8 };
  static constexpr float kSR = 48000.f;

  // dimensioni della diffusione (campioni)
  static constexpr uint32_t kTankL = 2203, kTankR = 2477, kTapL = 672, kTapR = 908;
  static constexpr uint32_t kVerbFloats = 142 + 107 + 379 + 277 + 149 + 113 + 389 + 283 + kTankL + kTankR + kTapL + kTapR;

  // ---------------------------------------------------------------------
  void init(float *buf, uint32_t size_pow2, float *verb) {
    buf_ = buf; size_ = size_pow2; mask_ = size_pow2 - 1; w_ = 0;
    float *p = verb;
    for (int i = 0; i < 8; ++i) { ap_[i] = p; p += kApLen[i]; ap_i_[i] = 0; }
    tankL_ = p; p += kTankL; tankR_ = p; p += kTankR; tapL_ = p; p += kTapL; tapR_ = p; p += kTapR;
    tank_i_[0] = tank_i_[1] = tap_i_[0] = tap_i_[1] = 0;
    clear();
    time_ = 0.39f; depth_ = 0.39f; mix_ = 0.f; mode_ = M_CLOUD; fb_ = 0.35f; space_ = 0.4f; tone_ = 0.8f;
    pitch_ = 3; sync_ = false; hold_ = false; spread_ = 0.6f; bpm_ = 120.f;
    rng_ = 0x2545F491u;
    for (int i = 0; i < MAX_GRAINS; ++i) g_[i].active = false;
    spawn_cnt_ = 0.f; loop_cycles_ = 0; since_capture_ = 0; loop_start_ = 0; loop_len_ = 0;
    tap_d_ = 24000.f;
  }

  void clear() {
    if (buf_) for (uint32_t i = 0; i < size_; ++i) buf_[i] = 0.f;
    for (int i = 0; i < 8; ++i) for (uint32_t k = 0; k < kApLen[i]; ++k) ap_[i][k] = 0.f;
    for (uint32_t k = 0; k < kTankL; ++k) tankL_[k] = 0.f;
    for (uint32_t k = 0; k < kTankR; ++k) tankR_[k] = 0.f;
    for (uint32_t k = 0; k < kTapL; ++k) tapL_[k] = 0.f;
    for (uint32_t k = 0; k < kTapR; ++k) tapR_[k] = 0.f;
    fbL_ = fbR_ = 0.f; lpL_ = lpR_ = 0.f; hp_x_ = hp_y_ = 0.f; dampL_ = dampR_ = 0.f; fb_lp_ = 0.f;
    for (int i = 0; i < MAX_GRAINS; ++i) g_[i].active = false;
  }

  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_TIME:   time_ = v * (1.f / 1023.f); break;
      case P_DEPTH:  depth_ = v * (1.f / 1023.f); break;
      case P_MIX:    mix_ = v * 0.001f; break;
      case P_MODE:   if ((uint8_t)v != mode_) { mode_ = (uint8_t)v; for (int i = 0; i < MAX_GRAINS; ++i) g_[i].active = false; spawn_cnt_ = 0.f; loop_len_ = 0; } break;
      case P_FB:     fb_ = v * 0.01f; break;
      case P_SPACE:  space_ = v * 0.01f; break;
      case P_TONE:   tone_ = v * 0.01f; break;
      case P_PITCH:  pitch_ = (uint8_t)v; break;
      case P_SYNC:   sync_ = (v != 0); break;
      case P_HOLD:   hold_ = (v != 0); break;
      case P_SPREAD: spread_ = v * 0.01f; break;
      default: break;
    }
  }

  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *modes[] = {"CLOUD", "LOOP", "REVERSE", "TAPS"};
    static const char *pitches[] = {"-12", "-7", "-5", "0", "5", "7", "12", "OCTAVES"};
    if (id == P_MODE && v >= 0 && v < 4) return modes[v];
    if (id == P_PITCH && v >= 0 && v < 8) return pitches[v];
    return nullptr;
  }

  void setTempo(float bpm) { if (bpm > 10.f && bpm < 1000.f) bpm_ = bpm; }

  // ---------------------------------------------------------------------
  void process(const float *in, float *out, uint32_t frames) {
    if (!buf_) { for (uint32_t i = 0; i < frames * 2; ++i) out[i] = in[i]; return; }

    // mix: centro = dry e wet pieni
    const float m = (mix_ + 1.f) * 0.5f;
    const float dry_g = m < 0.5f ? 1.f : 2.f * (1.f - m);
    const float wet_g = m > 0.5f ? 1.f : 2.f * m;

    // TONE: passa-basso 500 Hz .. 20 kHz (100% = escluso)
    const bool tone_on = tone_ < 0.995f;
    const float fc = 500.f * pow2(tone_ * 5.32f);              // 500 * 2^5.32 ~ 20 kHz
    float x = 6.2831853f * fc / kSR; if (x > 3.f) x = 3.f;
    const float tone_c = x / (1.f + x);

    const float fb = fb_ * 0.95f;
    const float verb_fb = 0.35f + space_ * 0.5f;                 // lunghezza coda
    const float verb_damp = 0.3f + (1.f - tone_) * 0.4f;

    const float ratio = pitchRatio(pitch_);

    for (uint32_t i = 0; i < frames; ++i) {
      const float inL = in[2 * i], inR = in[2 * i + 1];

      // --- scrittura nel buffer (salvo HOLD) ---
      if (!hold_) {
        float fbs = (fbL_ + fbR_) * 0.5f * fb;
        // passa-alto ~60 Hz nel loop di feedback
        const float hy = fbs - hp_x_ + 0.9922f * hp_y_; hp_x_ = fbs; hp_y_ = hy;
        float wv = (inL + inR) * 0.5f + hy;
        wv = softclip(wv);
        buf_[w_] = wv;
        w_ = (w_ + 1) & mask_;
        ++since_capture_;
      }

      // --- generazione wet ---
      float wl = 0.f, wr = 0.f;
      switch (mode_) {
        case M_CLOUD:   schedCloud(ratio); break;
        case M_LOOP:    schedLoop(ratio); break;
        case M_REVERSE: schedReverse(ratio); break;
        default: break;
      }
      if (mode_ == M_TAPS) renderTaps(wl, wr);
      else renderGrains(wl, wr);

      // TONE
      if (tone_on) { lpL_ += (wl - lpL_) * tone_c; lpR_ += (wr - lpR_) * tone_c; wl = lpL_; wr = lpR_; }

      if (mode_ == M_TAPS) { fbL_ = fbR_ = tap_fb_; }
      else { fbL_ = wl; fbR_ = wr; }

      // SPACE: diffusione + tank
      if (space_ > 0.001f) {
        float vl, vr;
        verb(wl, wr, vl, vr, verb_fb, verb_damp);
        wl = wl * (1.f - space_ * 0.6f) + vl * space_ * 1.1f;
        wr = wr * (1.f - space_ * 0.6f) + vr * space_ * 1.1f;
      }

      out[2 * i]     = inL * dry_g + softclip(wl * 1.3f) * wet_g;
      out[2 * i + 1] = inR * dry_g + softclip(wr * 1.3f) * wet_g;
    }
  }

 private:
  struct Grain {
    bool active;
    int32_t ip; float fp;      // posizione di lettura (intera + frazione)
    float rate;                // velocita' (negativa = al contrario)
    float len, age, fade;      // in campioni di uscita; fade = 0 -> finestra Hann
    float gl, gr;
    float ws, wc, wsd, wcd;    // finestra sin(pi*ph) per rotazione
  };

  // ---------- utilita' ----------
  static inline float pow2(float x) {     // 2^x, x >= 0
    const int32_t ip = (int32_t)x; const float f = x - ip;
    return (float)(1u << ip) * (1.f + f * (0.6931472f + f * (0.2402265f + f * 0.0555041f)));
  }
  static inline float softclip(float x) {
    if (x > 1.5f) return 1.f; if (x < -1.5f) return -1.f;
    return x - x * x * x * (4.f / 27.f);
  }
  inline uint32_t nextRand() { uint32_t r = rng_; r ^= r << 13; r ^= r >> 17; r ^= r << 5; rng_ = r; return r; }
  inline float rand01() { return (nextRand() >> 8) * (1.f / 16777216.f); }

  static float pitchRatio(uint8_t p) {
    static const float r[] = {0.5f, 0.6674199f, 0.7491535f, 1.f, 1.3348398f, 1.4983071f, 2.f, 1.f};
    return r[p < 8 ? p : 3];
  }
  inline float grainRatio(float base) {   // OCTAVES: scelta casuale tra -12, 0, +12
    if (pitch_ != 7) return base;
    const uint32_t k = nextRand() % 3;
    return k == 0 ? 0.5f : (k == 1 ? 1.f : 2.f);
  }

  // tempo in campioni: libero (esponenziale) o sincronizzato
  float timeSamples(float tmin_s, float tmax_s) const {
    if (sync_) {
      static const float q[] = {0.125f, 0.16667f, 0.25f, 0.33333f, 0.375f, 0.5f, 0.66667f, 0.75f, 1.f, 2.f, 4.f};
      int idx = (int)(time_ * 10.999f);
      float s = q[idx] * (60.f / bpm_) * kSR;
      const float mx = tmax_s * kSR, mn = tmin_s * kSR;
      while (s > mx) s *= 0.5f;
      while (s < mn) s *= 2.f;
      return s;
    }
    // log2(tmax/tmin) calcolato a mano per i casi usati
    const float span = tmax_s / tmin_s;
    float l2 = 0.f; float t = span; while (t >= 2.f) { t *= 0.5f; l2 += 1.f; } l2 += (t - 1.f) * 1.f;
    return tmin_s * kSR * pow2(time_ * l2);
  }

  inline float readAt(int32_t ip, float fp) const {
    const float a = buf_[(uint32_t)ip & mask_], b = buf_[(uint32_t)(ip + 1) & mask_];
    return a + (b - a) * fp;
  }

  void startGrain(int32_t delay, float rate, float len, float fade, float pan) {
    for (int k = 0; k < MAX_GRAINS; ++k) {
      Grain &g = g_[k];
      if (g.active) continue;
      g.active = true;
      g.ip = (int32_t)w_ - delay; g.fp = 0.f;
      g.rate = rate; g.len = len < 16.f ? 16.f : len; g.age = 0.f; g.fade = fade;
      { const float th = 3.14159265f / g.len, t2 = th * th;
        g.wsd = th * (1.f - t2 * (1.f / 6.f) * (1.f - t2 * 0.05f)); g.wcd = 1.f - t2 * 0.5f * (1.f - t2 * (1.f / 12.f));
        g.ws = 0.f; g.wc = 1.f; }
      // pan a potenza costante approssimata
      const float p = 0.5f + pan * 0.5f * spread_;
      g.gl = (1.f - p) * (1.f + p * 0.6f);
      g.gr = p * (1.f + (1.f - p) * 0.6f);
      return;
    }
  }

  void renderGrains(float &l, float &r) {
    int n = 0;
    for (int k = 0; k < MAX_GRAINS; ++k) {
      Grain &g = g_[k];
      if (!g.active) continue;
      ++n;
      float win;
      if (g.fade <= 0.f) {                       // Hann = sin^2, seno per rotazione (niente divisioni)
        const float s = g.ws;
        { const float ns = g.ws * g.wcd + g.wc * g.wsd; g.wc = g.wc * g.wcd - g.ws * g.wsd; g.ws = ns; }
        win = s * s;
      } else {                                   // trapezio (crossfade lineare)
        const float t = g.age, e = g.len - g.age;
        win = t < g.fade ? t / g.fade : (e < g.fade ? e / g.fade : 1.f);
      }
      const float s = readAt(g.ip, g.fp) * win;
      l += s * g.gl; r += s * g.gr;
      // avanza
      g.fp += g.rate;
      const int32_t d = (int32_t)g.fp - (g.fp < 0.f && (float)(int32_t)g.fp != g.fp ? 1 : 0);
      g.ip += d; g.fp -= (float)d;
      g.age += 1.f;
      if (g.age >= g.len) g.active = false;
    }
    (void)n;
    l *= grain_gain_; r *= grain_gain_;
  }

  // ---------- CLOUD ----------
  void schedCloud(float base) {
    const float len = timeSamples(0.03f, 0.5f);
    const float density = 2.f + depth_ * depth_ * 38.f;          // grani al secondo
    float overlap = density * len / kSR;
    if (overlap > MAX_GRAINS) overlap = MAX_GRAINS;
    grain_gain_ = 1.5f / sqrtApprox(overlap < 1.f ? 1.f : overlap);
    spawn_cnt_ -= 1.f;
    if (spawn_cnt_ > 0.f) return;
    spawn_cnt_ = kSR / density * (0.6f + 0.8f * rand01());      // jitter
    const float rate = grainRatio(base) * (1.f + (rand01() - 0.5f) * 0.012f);  // +/- 10 cents
    const float need = len * (rate > 1.f ? rate - 1.f : 0.f) + 64.f;
    const float scatter_max = hold_ ? kSR * 2.f : len * 3.f + kSR * 0.25f * depth_;
    int32_t delay = (int32_t)(need + rand01() * scatter_max);
    if (rate > 0.f && delay > (int32_t)size_ - (int32_t)(len * rate) - 64) delay = (int32_t)size_ - (int32_t)(len * rate) - 64;
    startGrain(delay, rate, len, 0.f, rand01() * 2.f - 1.f);
  }

  // ---------- LOOP ----------
  void schedLoop(float base) {
    grain_gain_ = 1.f;
    const float seg = timeSamples(0.04f, 2.f);
    spawn_cnt_ -= 1.f;
    if (spawn_cnt_ > 0.f) return;
    // ricattura: ogni 4 giri, o se il buffer rischia di sovrascrivere il loop
    if (loop_len_ == 0 || (!hold_ && (loop_cycles_ >= 4 || since_capture_ > size_ / 2))) {
      loop_len_ = (uint32_t)seg;
      loop_start_ = (w_ - loop_len_ - 32) & mask_;
      loop_cycles_ = 0; since_capture_ = 0;
    }
    ++loop_cycles_;
    float rate = grainRatio(base);
    float len = (float)loop_len_;
    int32_t start = (int32_t)loop_start_;
    bool rev = false;
    // variazioni (DEPTH = probabilita')
    if (rand01() < depth_ * 0.8f) {
      const uint32_t v = nextRand() % 4;
      if (v == 0) rev = true;                       // al contrario
      else if (v == 1) len *= 0.5f;                 // stutter: meta'
      else if (v == 2) rate *= 2.f;                 // ottava sopra
      else len *= 0.25f;                            // stutter: quarto
    }
    const float fade = 240.f;                       // 5 ms
    const float out_len = len / rate;
    const int32_t delay = rev ? (int32_t)((w_ - (uint32_t)start - (uint32_t)len) & mask_)
                              : (int32_t)((w_ - (uint32_t)start) & mask_);
    startGrain(delay, rev ? -rate : rate, out_len + fade, fade, (rand01() * 2.f - 1.f) * 0.5f);
    spawn_cnt_ = out_len;
  }

  // ---------- REVERSE ----------
  void schedReverse(float base) {
    grain_gain_ = 1.f;
    const float len = timeSamples(0.08f, 2.f);
    spawn_cnt_ -= 1.f;
    if (spawn_cnt_ > 0.f) return;
    spawn_cnt_ = len * 0.5f;                        // due grani Hann sovrapposti al 50%
    float rate = grainRatio(base);
    if (rand01() < depth_ * 0.5f) rate *= 2.f;      // DEPTH: ottave sopra casuali
    int32_t delay = 2 + (int32_t)(rand01() * depth_ * len);
    startGrain(delay, -rate, len, 0.f, (rand01() * 2.f - 1.f) * (0.3f + depth_ * 0.7f));
  }

  // ---------- TAPS ----------
  void renderTaps(float &l, float &r) {
    static const uint8_t pats[8] = {0x08, 0x88, 0x0C, 0x4A, 0x2A, 0x6D, 0xAA, 0xFF};
    const float target = timeSamples(0.05f, 1.f);
    tap_d_ += (target - tap_d_) * 0.0005f;         // cambio di tempo "a nastro"
    const uint8_t pat = pats[(int)(depth_ * 7.999f)];
    int cnt = 0;
    tap_fb_ = 0.f;
    for (int k = 1; k <= 8; ++k) {
      if (!((pat >> (k - 1)) & 1u)) continue;
      ++cnt;
      const float d = tap_d_ * k * 0.25f;
      const int32_t di = (int32_t)d; const float df = d - di;
      const int32_t ip = (int32_t)w_ - di - 1;
      const float raw = readAt(ip, 1.f - df);
      tap_fb_ = raw;                              // l'ultimo tap attivo (il piu' lungo) va in feedback
      const float s = raw * (1.f - 0.07f * k);
      const float pan = (k & 1) ? -spread_ : spread_;
      l += s * (1.f - pan) * 0.5f * 1.4f;
      r += s * (1.f + pan) * 0.5f * 1.4f;
    }
    const float n = cnt > 1 ? 1.f / sqrtApprox((float)cnt) : 1.f;
    l *= n; r *= n;
  }

  // ---------- diffusione riverberante ----------
  inline float allpass(int i, float x, float g) {
    float *b = ap_[i]; uint32_t &idx = ap_i_[i];
    const float d = b[idx];
    const float y = d - g * x;
    b[idx] = x + g * y;
    if (++idx >= kApLen[i]) idx = 0;
    return y;
  }
  void verb(float inl, float inr, float &ol, float &orr, float fbk, float damp) {
    float l = inl, r = inr;
    for (int i = 0; i < 4; ++i) l = allpass(i, l, 0.65f);
    for (int i = 4; i < 8; ++i) r = allpass(i, r, 0.65f);
    // tank incrociato L<->R
    const float tl = tankL_[tank_i_[0]], tr = tankR_[tank_i_[1]];
    dampL_ += (tl - dampL_) * (1.f - damp);
    dampR_ += (tr - dampR_) * (1.f - damp);
    float nl = l + dampR_ * fbk, nr = r + dampL_ * fbk;
    // allpass interno al tank
    {
      float d = tapL_[tap_i_[0]]; float y = d - 0.5f * nl; tapL_[tap_i_[0]] = nl + 0.5f * y; nl = y;
      if (++tap_i_[0] >= kTapL) tap_i_[0] = 0;
      d = tapR_[tap_i_[1]]; y = d - 0.5f * nr; tapR_[tap_i_[1]] = nr + 0.5f * y; nr = y;
      if (++tap_i_[1] >= kTapR) tap_i_[1] = 0;
    }
    tankL_[tank_i_[0]] = nl; tankR_[tank_i_[1]] = nr;
    if (++tank_i_[0] >= kTankL) tank_i_[0] = 0;
    if (++tank_i_[1] >= kTankR) tank_i_[1] = 0;
    ol = tl + l * 0.3f; orr = tr + r * 0.3f;
  }

  static inline float sqrtApprox(float x) {   // Newton, x in [1, 16]
    float y = 0.5f * (x + 1.f);
    y = 0.5f * (y + x / y); y = 0.5f * (y + x / y); y = 0.5f * (y + x / y);
    return y;
  }

  // buffer
  float *buf_ = nullptr; uint32_t size_ = 0, mask_ = 0, w_ = 0;
  float *ap_[8]; uint32_t ap_i_[8];
  float *tankL_, *tankR_, *tapL_, *tapR_; uint32_t tank_i_[2], tap_i_[2];
  // parametri
  float time_, depth_, mix_, fb_, space_, tone_, spread_, bpm_;
  uint8_t mode_, pitch_; bool sync_, hold_;
  // stato
  uint32_t rng_;
  Grain g_[MAX_GRAINS];
  float grain_gain_ = 1.f, spawn_cnt_;
  uint32_t loop_cycles_, since_capture_, loop_start_, loop_len_;
  float tap_d_, tap_fb_ = 0.f;
  float fbL_, fbR_, lpL_, lpR_, hp_x_, hp_y_, dampL_, dampR_, fb_lp_;
};
