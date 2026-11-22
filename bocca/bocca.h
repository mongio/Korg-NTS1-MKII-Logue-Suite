#pragma once
/*
 *  File: bocca.h  (BOCCA v0.2: coefficienti a blocchi di 4 campioni, un solo canale se l'ingresso e' mono)
 *  Filtro a formanti stereo: 3 filtri passa-banda (TPT SVF) per canale sulle formanti F1-F3,
 *  morphing continuo tra le vocali U-O-A-E-I, mosso da LFO, caso, inviluppo, registro Turing
 *  o da un "chiacchiericcio" sillabico. Nessuna dipendenza da libm.
 */
#include <stdint.h>

class Bocca {
 public:
  enum { P_VOWEL = 0, P_DEPTH, P_MOD, P_RATE, P_VOICE, P_RESO, P_GLIDE, P_DRIVE, P_BODY, P_MIX, P_NUM };
  enum { M_LFO = 0, M_RANDOM, M_ENV, M_TURING, M_LOOP, M_CHAT };
  enum { RATE_FREE = 50, RATE_MAX = 60 };
  static constexpr float kSR = 48000.f;

  void init() {
    vowel_ = 0.5f; depth_ = 0.6f; mod_ = M_LFO; rate_i_ = 30; voice_ = 0.5f; reso_ = 0.5f;
    glide_ = 0.15f; drive_ = 0.2f; body_ = 0.3f; mix_ = 1.f; bpm_ = 120.f;
    rng_ = 0x424F4343u; reg_ = 0xA5C3u;
    clear();
  }
  void clear() {
    for (int c = 0; c < 2; ++c) for (int f = 0; f < 3; ++f) { ic1_[c][f] = ic2_[c][f] = 0.f; }
    ph_ = 0.f; pos_ = 2.f; env_ = 0.f; rnd_v_ = rnd_t_ = 0.f; step_cnt_ = 0.f; mval_ = 0.5f;
    chat_gain_ = 1.f; chat_dip_ = 0; dcx_[0] = dcx_[1] = dcy_[0] = dcy_[1] = 0.f;
  }

  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_VOWEL: vowel_ = v * (1.f / 1023.f); break;
      case P_DEPTH: depth_ = v * (1.f / 1023.f); break;
      case P_MOD:   mod_ = (uint8_t)v; break;
      case P_RATE:  rate_i_ = (uint8_t)v; break;
      case P_VOICE: voice_ = v * 0.01f; break;
      case P_RESO:  reso_ = v * 0.01f; break;
      case P_GLIDE: glide_ = v * 0.01f; break;
      case P_DRIVE: drive_ = v * 0.01f; break;
      case P_BODY:  body_ = v * 0.01f; break;
      case P_MIX:   mix_ = v * 0.01f; break;
      default: break;
    }
  }

  const char *paramStr(uint8_t id, int32_t v) {
    static const char *mods[] = {"LFO", "RANDOM", "ENV", "TURING", "LOOP", "CHAT"};
    static const char *divs[] = {"1-1", "1-2", "1-4.", "1-4", "1-4T", "1-8.", "1-8", "1-8T", "1-16", "1-16T", "1-32"};
    if (id == P_MOD && v >= 0 && v < 6) return mods[v];
    if (id == P_RATE) {
      if (v >= RATE_FREE && v <= RATE_MAX) return divs[v - RATE_FREE];
      return fmtHz(freeRate((uint8_t)v));
    }
    return nullptr;
  }

  void setTempo(float bpm) { if (bpm > 10.f && bpm < 1000.f) bpm_ = bpm; }

  void process(const float *in, float *out, uint32_t frames) {
    // vocali (Hz, voce adulta media): U O A E I
    static const float F1[5] = {325.f, 450.f, 700.f, 530.f, 300.f};
    static const float F2[5] = {700.f, 800.f, 1220.f, 1840.f, 2300.f};
    static const float F3[5] = {2530.f, 2830.f, 2600.f, 2480.f, 3000.f};
    static const float G2[5] = {0.35f, 0.45f, 0.6f, 0.55f, 0.5f};     // peso di F2 per vocale
    static const float GV[5] = {2.3f, 3.9f, 4.5f, 3.9f, 1.9f};        // compensazione di livello per vocale (misurata)
    // frequenza della modulazione: libera 0,05..12 Hz o suddivisione del tempo
    float hz;
    if (rate_i_ >= RATE_FREE) {
      static const float q[] = {4.f, 2.f, 1.5f, 1.f, 0.6667f, 0.75f, 0.5f, 0.3333f, 0.25f, 0.1667f, 0.125f};   // in quarti
      hz = bpm_ / 60.f / q[rate_i_ - RATE_FREE];
    } else hz = freeRate(rate_i_);
    const float dph = hz / kSR;
    const float vscale = 0.78f + voice_ * 0.6f;                         // 0.78x (uomo) .. 1.38x (bambino)
    const float qmul = 0.5f + reso_ * 2.f;
    const float qcomp = sqrtApprox(qmul) * 1.25f;                    // filtri piu' stretti = meno energia: compensa
    const float glide_c = 1.f / (1.f + glide_ * glide_ * 14400.f);    // fino a ~300 ms
    const float drv = 1.f + drive_ * 6.f, drv_mk = (1.f + 0.45f * (drv - 1.f)) / drv;
    const float body = body_ * 0.6f;

    // ingresso mono (caso tipico: l'oscillatore del NTS-1)? Allora si calcola un solo canale e si copia
    bool mono = true;
    for (uint32_t k = 0; k < frames && mono; ++k) if (in[2 * k] != in[2 * k + 1]) mono = false;
    if (mono && !was_mono_) { /* nulla da fare: lo stato del canale 0 prosegue */ }
    if (!mono && was_mono_) {               // ritorno allo stereo: il canale destro riparte dallo stato del sinistro
      for (int f = 0; f < 3; ++f) { ic1_[1][f] = ic1_[0][f]; ic2_[1][f] = ic2_[0][f]; }
      dcx_[1] = dcx_[0]; dcy_[1] = dcy_[0];
    }
    was_mono_ = mono;
    const int nch = mono ? 1 : 2;
    for (uint32_t n = 0; n < frames; ++n) {
      const float inL = in[2 * n], inR = in[2 * n + 1];

      // ---- sorgente di modulazione: mval_ in 0..1 ----
      const float a = absf(inL) + absf(inR);
      env_ += (a - env_) * (a > env_ ? 0.004f : 0.0004f);              // attacco ~5 ms, rilascio ~50 ms
      bool step = false;
      ph_ += dph; if (ph_ >= 1.f) { ph_ -= 1.f; step = true; }
      switch (mod_) {
        case M_LFO:    mval_ = 0.5f + 0.5f * sinp(ph_); break;
        case M_RANDOM: if (step) rnd_t_ = rand01(); rnd_v_ += (rnd_t_ - rnd_v_) * (dph * 4.f > 0.02f ? 0.02f : dph * 4.f); mval_ = rnd_v_; break;
        case M_ENV:    { float e = env_ * 2.5f; mval_ = e > 1.f ? 1.f : e; } break;
        case M_TURING:
        case M_LOOP:
          if (step) {
            uint32_t bit = (reg_ >> 7) & 1u;
            if (mod_ == M_TURING && rand01() < 0.25f) bit ^= 1u;      // muta; LOOP: 8 passi bloccati
            reg_ = ((reg_ << 1) | bit) & 0xFFFFu;
            mval_ = (float)(reg_ & 0x7u) * (1.f / 7.f);                // 8 livelli -> vocali a gradini
          } break;
        case M_CHAT:
          if (step) {                                                  // nuova sillaba
            mval_ = rand01();
            ph_ = rand01() * 0.4f;                                     // sillabe di durata irregolare
            if (rand01() < 0.55f) chat_dip_ = (int32_t)(kSR * (0.012f + 0.025f * rand01()));   // consonante
          } break;
        default: break;
      }
      // chiusura breve da consonante (solo CHAT)
      float cg = 1.f;
      if (mod_ == M_CHAT) { if (chat_dip_ > 0) { --chat_dip_; cg = 0.25f; } }
      chat_gain_ += (cg - chat_gain_) * 0.01f;

      // ---- posizione della bocca (0 = U ... 4 = I) ----
      float target;
      if (mod_ == M_ENV) target = vowel_ * 4.f + depth_ * 4.f * mval_ - depth_ * 1.f;   // si apre col volume
      else target = vowel_ * 4.f + depth_ * 2.5f * (2.f * mval_ - 1.f);
      target = target < 0.f ? 0.f : (target > 4.f ? 4.f : target);
      const float gc = (mod_ == M_TURING || mod_ == M_LOOP || mod_ == M_CHAT) ? glide_c : (glide_c > 0.05f ? 0.05f : glide_c);
      pos_ += (target - pos_) * gc;

      // ---- formanti interpolate ----
      const int32_t i0 = (int32_t)pos_ >= 4 ? 3 : (int32_t)pos_;
      const float fr = pos_ - (float)i0;
      const float f1 = lerp(F1[i0], F1[i0 + 1], fr) * vscale;
      const float f2 = lerp(F2[i0], F2[i0 + 1], fr) * vscale;
      const float f3 = lerp(F3[i0], F3[i0 + 1], fr) * vscale;
      const float g2 = lerp(G2[i0], G2[i0 + 1], fr);
      const float gv = lerp(GV[i0], GV[i0 + 1], fr) * qcomp;
      // larghezze di banda tipiche: 80 / 100 / 140 Hz (RESO le stringe o allarga); la selettivita' non dipende da VOICE
      const float iv = 1.f / vscale;
      // coefficienti ricalcolati ogni 4 campioni (la bocca si muove molto piu' lentamente)
      if ((n & 3u) == 0u) { c1_ = coef(f1, f1 * iv / 80.f * qmul); c2_ = coef(f2, f2 * iv / 100.f * qmul); c3_ = coef(f3, f3 * iv / 140.f * qmul); gv_ = gv; g2_ = g2; }
      const Coef &c1 = c1_, &c2 = c2_, &c3 = c3_;

      float o[2];
      const float xin[2] = {inL, inR};
      for (int ch = 0; ch < nch; ++ch) {
        float x = xin[ch];
        if (drive_ > 0.f) { const float z = x * drv; x = z / (1.f + absf(z)) * drv_mk; }
        const float b1 = band(ch, 0, c1, x), b2 = band(ch, 1, c2, x), b3 = band(ch, 2, c3, x);
        float y = (b1 + b2 * g2_ + b3 * 0.3f) * gv_ + x * body;
        y *= chat_gain_;
        // blocco DC e limitatore morbido
        const float dy = y - dcx_[ch] + 0.9975f * dcy_[ch]; dcx_[ch] = y; dcy_[ch] = dy; y = dy;
        const float ay = absf(y);
        if (ay > 0.8f) { const float e = (ay - 0.8f) * 5.f; const float l = 0.8f + 0.2f * e / (1.f + e); y = y < 0.f ? -l : l; }
        o[ch] = y;
      }
      if (mono) o[1] = o[0];
      out[2 * n]     = inL * (1.f - mix_) + o[0] * mix_;
      out[2 * n + 1] = inR * (1.f - mix_) + o[1] * mix_;
    }
  }

 private:
  struct Coef { float a1, a2, a3, k; };
  static Coef coef(float fc, float q) {
    if (fc > 16000.f) fc = 16000.f;
    if (q < 0.7f) q = 0.7f;
    const float x = 3.14159265f * fc / kSR, x2 = x * x;
    const float g = x * (1.f + x2 * (0.3333333f + x2 * (0.1333333f + x2 * 0.0539683f)));   // tan(x)
    Coef c; c.k = 1.f / q; c.a1 = 1.f / (1.f + g * (g + c.k)); c.a2 = g * c.a1; c.a3 = g * c.a2;
    return c;
  }
  // passa-banda normalizzato (guadagno 1 al centro)
  inline float band(int ch, int f, const Coef &c, float v0) {
    float &i1 = ic1_[ch][f], &i2 = ic2_[ch][f];
    const float v3 = v0 - i2, v1 = c.a1 * i1 + c.a2 * v3, v2 = i2 + c.a2 * i1 + c.a3 * v3;
    i1 = 2.f * v1 - i1; i2 = 2.f * v2 - i2;
    return v1 * c.k;
  }

  static inline float absf(float x) { return x < 0.f ? -x : x; }
  static inline float sqrtApprox(float x) { float y = 0.5f * (x + 1.f); for (int k = 0; k < 4; ++k) y = 0.5f * (y + x / y); return y; }
  static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
  static inline float sinp(float ph) { float x = ph < 0.5f ? ph * 4.f - 1.f : 3.f - ph * 4.f; return x * (1.5f - 0.5f * x * x); }
  static inline float pow2(float x) { const int32_t ip = (int32_t)x; const float f = x - ip;
    return (float)(1u << ip) * (1.f + f * (0.6931472f + f * (0.2402265f + f * 0.0555041f))); }
  static float freeRate(uint8_t i) { return 0.05f * pow2((float)i * (7.907f / 49.f)); }   // 0,05 .. 12 Hz
  inline uint32_t nextRand() { uint32_t r = rng_; r ^= r << 13; r ^= r >> 17; r ^= r << 5; rng_ = r; return r; }
  inline float rand01() { return (nextRand() >> 8) * (1.f / 16777216.f); }

  // formatta Hz senza printf: "0.05" .. "12"
  const char *fmtHz(float hz) {
    char *p = buf_;
    int32_t c = (int32_t)(hz * 100.f + 0.5f);       // centesimi
    if (hz >= 10.f) { const int32_t i = (c + 50) / 100; *p++ = (char)('0' + i / 10); *p++ = (char)('0' + i % 10); }
    else if (hz >= 1.f) { const int32_t d = (c + 5) / 10; *p++ = (char)('0' + d / 10); *p++ = '.'; *p++ = (char)('0' + d % 10); }
    else { *p++ = '.'; *p++ = (char)('0' + (c / 10) % 10); *p++ = (char)('0' + c % 10); }
    *p = 0;
    return buf_;
  }

  float vowel_, depth_, voice_, reso_, glide_, drive_, body_, mix_, bpm_;
  uint8_t mod_, rate_i_;
  uint32_t rng_, reg_;
  float ic1_[2][3], ic2_[2][3];
  float ph_, pos_, env_, rnd_v_, rnd_t_, step_cnt_, mval_, chat_gain_;
  int32_t chat_dip_;
  float dcx_[2], dcy_[2];
  char buf_[8];
  Coef c1_, c2_, c3_; float gv_ = 1.f, g2_ = 0.5f; bool was_mono_ = true;
};
