#pragma once
/*
 *  SUPERPHASER v0.1 - phaser vintage con ensemble "string machine":
 *  ENSEMBLE (tre linee BBD modulate da un LFO lento e uno veloce, sfasate di 120 gradi) -> PHASER a 4..12 stadi
 *  (stadi leggermente diversi come i circuiti veri, sweep esponenziale, risonanza COLOR con saturazione morbida).
 *  Nessuna dipendenza da libm.
 */
#include <stdint.h>
class Phaser {
 public:
  enum { P_RATE = 0, P_DEPTH, P_STAGES, P_COLOR, P_CENTER, P_ENSEMBLE, P_STEREO, P_SHAPE, P_WARMTH, P_MIX, P_NUM };
  enum { S_SINE = 0, S_TRI, S_RANDOM };
  enum { N = 1024, MASK = N - 1, MAXST = 12 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kFloats = N;

  void init(float *mem) { eb_ = mem; clear();
    rate_ = 0.25f; depth_ = 0.7f; stages_ = 2; color_ = 0.45f; center_ = 0.45f; ens_ = 0.5f; stereo_ = 0.5f; shape_ = S_SINE; warm_ = 0.3f; mix_ = 0.5f; }
  void clear() { for (uint32_t i = 0; i < N; ++i) eb_[i] = 0.f; w_ = 0; ph_ = 0.f; es_ = ef_ = 0.f;
    for (int c = 0; c < 2; ++c) { for (int k = 0; k < MAXST; ++k) s_[c][k] = 0.f; fb_[c] = 0.f; lp_[c] = 0.f; a_[c] = 0.f; }
    rv_ = rt_ = 0.5f; rn_ = 0; rng_ = 0x50A11A55u; cnt_ = 0; }
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_RATE:     rate_ = v * (1.f / 1023.f); break;
      case P_DEPTH:    depth_ = v * (1.f / 1023.f); break;
      case P_STAGES:   stages_ = (uint8_t)v; break;
      case P_COLOR:    color_ = v * 0.01f; break;
      case P_CENTER:   center_ = v * 0.01f; break;
      case P_ENSEMBLE: ens_ = v * 0.01f; break;
      case P_STEREO:   stereo_ = v * 0.01f; break;
      case P_SHAPE:    shape_ = (uint8_t)v; break;
      case P_WARMTH:   warm_ = v * 0.01f; break;
      case P_MIX:      mix_ = v * 0.01f; break;
      default: break;
    }
  }
  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *st[] = {"4", "6", "8", "10", "12"};
    static const char *sh[] = {"SINE", "TRI", "RANDOM"};
    if (id == P_STAGES && v >= 0 && v < 5) return st[v];
    if (id == P_SHAPE && v >= 0 && v < 3) return sh[v];
    return nullptr;
  }

  void process(const float *in, float *out, uint32_t frames) {
    static const float kSpread[MAXST] = {1.00f, 1.07f, 0.94f, 1.12f, 0.90f, 1.05f, 0.97f, 1.10f, 0.92f, 1.03f, 0.96f, 1.08f};
    const int nst = 4 + 2 * (stages_ < 5 ? stages_ : 4);
    const float hz = 0.02f * pow2(rate_ * 8.29f);                       // 0,02 .. 6 Hz
    const float dph = hz / kSR;
    const float fc0 = 100.f * pow2(center_ * 5.3f);                     // centro dello sweep 100 Hz .. ~4 kHz
    const float oct = depth_ * 2.6f;                                    // escursione fino a +/- 2,6 ottave
    const float fbk = color_ * 0.82f;
    const float m = mix_;
    const float mk = 1.f / sqrtf_((1.f - m) * (1.f - m) + m * m);
    float x = 6.2831853f * (16000.f - warm_ * 9000.f) / kSR; const float wc = x / (1.f + x);
    const float drive = 1.f + warm_ * 1.2f;
    const float ens = ens_;
    for (uint32_t n = 0; n < frames; ++n) {
      float inL = in[2 * n], inR = in[2 * n + 1];
      // --- ENSEMBLE: tre voci BBD, LFO lento (0,63 Hz) + veloce (6,4 Hz), fasi 0/120/240 ---
      float eL = inL, eR = inR;
      if (ens > 0.005f) {
        eb_[w_] = (inL + inR) * 0.5f;
        es_ += 0.63f / kSR; if (es_ >= 1.f) es_ -= 1.f;
        ef_ += 6.4f / kSR; if (ef_ >= 1.f) ef_ -= 1.f;
        float v[3];
        for (int k = 0; k < 3; ++k) {
          const float o = k * (1.f / 3.f);
          const float d = (7.f + 1.9f * sinp(wrap(es_ + o)) + 0.22f * sinp(wrap(ef_ + o))) * 48.f;
          v[k] = readH(eb_, d);
        }
        const float cL = (v[0] + 0.7f * v[1] + 0.3f * v[2]) * 0.5f, cR = (v[2] + 0.7f * v[1] + 0.3f * v[0]) * 0.5f;
        eL = inL * (1.f - 0.6f * ens) + cL * ens * 1.1f;
        eR = inR * (1.f - 0.6f * ens) + cR * ens * 1.1f;
        w_ = (w_ + 1) & MASK;
      }
      // --- LFO del phaser (coefficienti aggiornati ogni 4 campioni) ---
      ph_ += dph; if (ph_ >= 1.f) ph_ -= 1.f;
      if ((cnt_++ & 3u) == 0u) {
        float lL, lR;
        if (shape_ == S_RANDOM) {
          if (--rn_ <= 0) { rn_ = (int32_t)(kSR / 4.f / (hz * 3.f + 0.05f)); rt_ = rand01() * 2.f - 1.f; }
          rv_ += (rt_ - rv_) * (hz * 0.004f + 0.0002f); lL = rv_; lR = rv_ * (1.f - 2.f * stereo_);
        } else if (shape_ == S_TRI) { lL = tri(ph_); lR = tri(wrap(ph_ + stereo_ * 0.5f)); }
        else { lL = sinp(ph_); lR = sinp(wrap(ph_ + stereo_ * 0.5f)); }
        const float fL = fc0 * pow2s(oct * lL), fR = fc0 * pow2s(oct * lR);   // sweep esponenziale (il "whoosh" analogico)
        a_[0] = coefA(fL); a_[1] = coefA(fR);
      }
      // --- PHASER: nst stadi passa-tutto del primo ordine per canale, con risonanza ---
      float y[2]; const float xin[2] = {eL, eR};
      for (int c = 0; c < 2; ++c) {
        float v = xin[c] * drive + soft(fb_[c]) * fbk;
        v = soft(v) / drive;
        const float base = a_[c];
        for (int k = 0; k < nst; ++k) {
          const float a = base + (kSpread[k] - 1.f) * 0.18f * (1.f - (base < 0.f ? -base : base));   // stadi non identici
          const float yk = a * v + s_[c][k];
          s_[c][k] = v - a * yk;
          v = yk;
        }
        fb_[c] = v;
        lp_[c] += (v - lp_[c]) * wc;
        y[c] = lp_[c];
      }
      out[2 * n]     = (eL * (1.f - m) + y[0] * m) * mk;
      out[2 * n + 1] = (eR * (1.f - m) + y[1] * m) * mk;
    }
  }

 private:
  // coefficiente del passa-tutto: a = (t - 1) / (t + 1), t = tan(pi f / fs)
  static inline float coefA(float f) {
    if (f > 18000.f) f = 18000.f; if (f < 20.f) f = 20.f;
    const float x = 3.14159265f * f / kSR, x2 = x * x;
    const float t = x * (1.f + x2 * (0.3333333f + x2 * (0.1333333f + x2 * 0.0539683f)));
    return (t - 1.f) / (t + 1.f);
  }
  inline float readH(const float *b, float d) const {
    const float rp = (float)w_ - d; int32_t i = (int32_t)rp; if (rp < 0.f && (float)i != rp) --i; const float f = rp - (float)i;
    const float xm1 = b[(uint32_t)(i - 1) & MASK], x0 = b[(uint32_t)i & MASK], x1 = b[(uint32_t)(i + 1) & MASK], x2 = b[(uint32_t)(i + 2) & MASK];
    const float c1 = 0.5f * (x1 - xm1), c2 = xm1 - 2.5f * x0 + 2.f * x1 - 0.5f * x2, c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
    return ((c3 * f + c2) * f + c1) * f + x0;
  }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }
  static inline float wrap(float p) { return p >= 1.f ? p - 1.f : p; }
  static inline float sinp(float ph) { float x = ph < 0.5f ? ph * 4.f - 1.f : 3.f - ph * 4.f; return x * (1.5f - 0.5f * x * x); }
  static inline float tri(float p) { return p < 0.5f ? 4.f * p - 1.f : 3.f - 4.f * p; }
  static inline float pow2(float x) { const int32_t ip = (int32_t)x; const float f = x - ip; return (float)(1u << ip) * (1.f + f * (0.6931472f + f * (0.2402265f + f * 0.0555041f))); }
  static inline float pow2s(float x) { return x >= 0.f ? pow2(x) : 1.f / pow2(-x); }
  static inline float sqrtf_(float x) { float y = x > 0.25f ? x : 0.5f; for (int k = 0; k < 5; ++k) y = 0.5f * (y + x / y); return y; }
  inline uint32_t nextRand() { uint32_t r = rng_; r ^= r << 13; r ^= r >> 17; r ^= r << 5; rng_ = r; return r; }
  inline float rand01() { return (nextRand() >> 8) * (1.f / 16777216.f); }
  float *eb_ = nullptr; uint32_t w_ = 0, rng_, cnt_ = 0;
  float rate_, depth_, color_, center_, ens_, stereo_, warm_, mix_; uint8_t stages_, shape_;
  float ph_, es_, ef_, s_[2][MAXST], fb_[2], lp_[2], a_[2], rv_, rt_; int32_t rn_;
};
