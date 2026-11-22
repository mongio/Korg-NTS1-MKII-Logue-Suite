#pragma once
/*
 *  SUPERFLANGER v0.1 - flanger stereo con 4 caratteri:
 *  CLASSIC (BBD con feedback), TAPE (attraverso lo zero, come due registratori a nastro),
 *  BARBUP / BARBDN (barberpole: tre linee a dente di sega con dissolvenze, sale o scende all'infinito).
 *  Interpolazione cubica di Hermite. Nessuna dipendenza da libm.
 */
#include <stdint.h>
class Flanger {
 public:
  enum { P_RATE = 0, P_DEPTH, P_MODE, P_FB, P_COLOR, P_MANUAL, P_STEREO, P_SHAPE, P_AGE, P_MIX, P_NUM };
  enum { M_CLASSIC = 0, M_TAPE, M_BARBUP, M_BARBDN };
  enum { S_SINE = 0, S_TRI, S_RANDOM, S_ENV };
  enum { N = 2048, MASK = N - 1 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kFloats = 2 * N;

  void init(float *mem) { bL_ = mem; bR_ = mem + N; clear();
    rate_ = 0.3f; depth_ = 0.6f; mode_ = M_CLASSIC; fb_ = 0.5f; neg_ = false; manual_ = 0.3f; stereo_ = 0.5f; shape_ = S_TRI; age_ = 0.2f; mix_ = 0.5f; }
  void clear() { for (uint32_t i = 0; i < kFloats; ++i) bL_[i] = 0.f; w_ = 0; ph_ = 0.f; for (int k = 0; k < 3; ++k) bph_[k] = k / 3.f;
    fbL_ = fbR_ = 0.f; lpL_ = lpR_ = 0.f; env_ = 0.f; rv_ = rt_ = 0.f; rn_ = 0; dL_ = dR_ = 0.f; rng_ = 0x1F1A2B3Cu; }
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_RATE:   rate_ = v * (1.f / 1023.f); break;
      case P_DEPTH:  depth_ = v * (1.f / 1023.f); break;
      case P_MODE:   mode_ = (uint8_t)v; break;
      case P_FB:     fb_ = v * 0.01f; break;
      case P_COLOR:  neg_ = (v != 0); break;
      case P_MANUAL: manual_ = v * 0.01f; break;
      case P_STEREO: stereo_ = v * 0.01f; break;
      case P_SHAPE:  shape_ = (uint8_t)v; break;
      case P_AGE:    age_ = v * 0.01f; break;
      case P_MIX:    mix_ = v * 0.01f; break;
      default: break;
    }
  }
  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *modes[] = {"CLASSIC", "TAPE", "BARBUP", "BARBDN"};
    static const char *cols[] = {"POS", "NEG"};
    static const char *shapes[] = {"SINE", "TRI", "RANDOM", "ENV"};
    if (id == P_MODE && v >= 0 && v < 4) return modes[v];
    if (id == P_COLOR && v >= 0 && v < 2) return cols[v];
    if (id == P_SHAPE && v >= 0 && v < 4) return shapes[v];
    return nullptr;
  }

  void process(const float *in, float *out, uint32_t frames) {
    const float hz = 0.02f * pow2(rate_ * 8.97f);                     // 0,02 .. 10 Hz
    const float dph = hz / kSR;
    const float fb = fb_ * 0.95f * (neg_ ? -1.f : 1.f);
    float x = 6.2831853f * (14000.f * pow2neg(age_ * 2.2f)) / kSR; const float ac = x / (1.f + x);   // AGE: 14 -> ~3 kHz
    const float sat = 1.f + age_ * 1.5f;
    const float m = mix_;
    const float mk = 1.f / sqrtf_(((1.f - m) * (1.f - m) + m * m));   // stesso volume del bypass (le due copie sono quasi scorrelate)
    // ritardi (in campioni): MANUAL = punto di partenza 0,2..8 ms, DEPTH = escursione fino a 7 ms
    const float dmin = (0.2f + manual_ * 7.8f) * 48.f, dexc = depth_ * 7.f * 48.f;
    const float t0 = 3.f * 48.f;                                       // TAPE: il dry e' ritardato di 3 ms
    for (uint32_t n = 0; n < frames; ++n) {
      const float inL = in[2 * n], inR = in[2 * n + 1];
      // --- LFO ---
      ph_ += dph; if (ph_ >= 1.f) ph_ -= 1.f;
      float lL, lR;   // 0..1
      switch (shape_) {
        case S_SINE: lL = 0.5f + 0.5f * sinp(ph_); lR = 0.5f + 0.5f * sinp(wrap(ph_ + stereo_ * 0.5f)); break;
        case S_RANDOM: { if (--rn_ <= 0) { rn_ = (int32_t)(kSR / (hz * 4.f + 0.05f)); rt_ = rand01(); } rv_ += (rt_ - rv_) * (hz * 0.0005f + 0.00002f); lL = rv_; lR = lL * (1.f - stereo_) + (1.f - lL) * stereo_; } break;
        case S_ENV: { const float a = (inL < 0.f ? -inL : inL) + (inR < 0.f ? -inR : inR); env_ += (a - env_) * (a > env_ ? 0.003f : 0.0003f); float e = env_ * 2.f; e = e > 1.f ? 1.f : e; lL = e; lR = e; } break;
        default: lL = tri01(ph_); lR = tri01(wrap(ph_ + stereo_ * 0.5f)); break;
      }
      float yL, yR;
      if (mode_ == M_BARBUP || mode_ == M_BARBDN) {
        // --- barberpole: tre testine a dente di sega sfasate di 1/3, finestre sin^2 che sommano a 1 ---
        float sL = 0.f, sR = 0.f;
        for (int k = 0; k < 3; ++k) {
          bph_[k] += dph * 0.5f; if (bph_[k] >= 1.f) bph_[k] -= 1.f;
          const float r = (mode_ == M_BARBUP) ? 1.f - bph_[k] : bph_[k];   // ritardo che cala = le tacche salgono
          const float d = dmin + (dexc + 48.f) * r;
          const float u = bph_[k] * (1.f - bph_[k]); const float s = 16.f * u / (5.f - 4.f * u); const float wgt = s * s * (2.f / 3.f);          // tre sin^2 sfasate di 1/3 sommano a 1,5
          const float off = (k == 1 ? stereo_ * 0.3f : 0.f);
          sL += readH(bL_, d) * wgt; sR += readH(bR_, d * (1.f + off * 0.05f)) * wgt;
        }
        yL = sL; yR = sR;
        writeLR(inL + yL * fb, inR + yR * fb, sat);
        out[2 * n] = (inL * (1.f - m) + age(yL, lpL_, ac) * m) * mk;
        out[2 * n + 1] = (inR * (1.f - m) + age(yR, lpR_, ac) * m) * mk;
      } else if (mode_ == M_TAPE) {
        // --- attraverso lo zero: dry fisso a 3 ms, wet che passa da 0 a 6 ms ---
        const float dL = t0 * (1.f + (2.f * lL - 1.f) * (0.3f + depth_ * 0.7f));
        const float dR = t0 * (1.f + (2.f * lR - 1.f) * (0.3f + depth_ * 0.7f));
        dL_ += (dL - dL_) * 0.01f; dR_ += (dR - dR_) * 0.01f;
        yL = readH(bL_, dL_ < 1.f ? 1.f : dL_); yR = readH(bR_, dR_ < 1.f ? 1.f : dR_);
        const float drL = readH(bL_, t0), drR = readH(bR_, t0);
        // COLOR NEG: copia invertita -> cancellazione totale quando le due copie si incrociano
        const float pol = neg_ ? -1.f : 1.f; yL *= pol; yR *= pol;
        writeLR(inL + yL * fb_ * 0.95f * 0.5f, inR + yR * fb_ * 0.95f * 0.5f, sat);   // (polarita' gia' nella copia)
        // due "registratori" uguali: AGE scurisce entrambe le copie, cosi' all'incrocio si annullano davvero
        out[2 * n] = (age(drL, dlL_, ac) * (1.f - m) + age(yL, lpL_, ac) * m) * mk;
        out[2 * n + 1] = (age(drR, dlR_, ac) * (1.f - m) + age(yR, lpR_, ac) * m) * mk;
      } else {
        // --- CLASSIC (BBD): ritardo = MANUAL + DEPTH * LFO ---
        const float dL = dmin + dexc * lL, dR = dmin + dexc * lR;
        dL_ += (dL - dL_) * 0.01f; dR_ += (dR - dR_) * 0.01f;
        yL = readH(bL_, dL_); yR = readH(bR_, dR_);
        writeLR(inL + yL * fb, inR + yR * fb, sat);
        out[2 * n] = (inL * (1.f - m) + age(yL, lpL_, ac) * m) * mk;
        out[2 * n + 1] = (inR * (1.f - m) + age(yR, lpR_, ac) * m) * mk;
      }
      w_ = (w_ + 1) & MASK;
    }
  }

 private:
  inline void writeLR(float l, float r, float sat) { bL_[w_] = soft(l * sat) / sat; bR_[w_] = soft(r * sat) / sat; }
  // Hermite a 4 punti: legge d campioni nel passato (d >= 1)
  inline float readH(const float *b, float d) const {
    const float rp = (float)w_ - d; int32_t i = (int32_t)rp; if (rp < 0.f && (float)i != rp) --i; const float f = rp - (float)i;
    const float xm1 = b[(uint32_t)(i - 1) & MASK], x0 = b[(uint32_t)i & MASK], x1 = b[(uint32_t)(i + 1) & MASK], x2 = b[(uint32_t)(i + 2) & MASK];
    const float c1 = 0.5f * (x1 - xm1), c2 = xm1 - 2.5f * x0 + 2.f * x1 - 0.5f * x2, c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
    return ((c3 * f + c2) * f + c1) * f + x0;
  }
  inline float age(float y, float &lp, float c) const { if (age_ < 0.01f) return y; lp += (y - lp) * c; return lp; }   // AGE a zero: escluso
  static inline float sqrtf_(float x) { float y = x > 0.25f ? x : 0.5f; for (int k = 0; k < 5; ++k) y = 0.5f * (y + x / y); return y; }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }
  static inline float wrap(float p) { return p >= 1.f ? p - 1.f : p; }
  static inline float sinp(float ph) { float x = ph < 0.5f ? ph * 4.f - 1.f : 3.f - ph * 4.f; return x * (1.5f - 0.5f * x * x); }
  static inline float tri01(float p) { return p < 0.5f ? 2.f * p : 2.f - 2.f * p; }
  static inline float pow2(float x) { const int32_t ip = (int32_t)x; const float f = x - ip; return (float)(1u << ip) * (1.f + f * (0.6931472f + f * (0.2402265f + f * 0.0555041f))); }
  static inline float pow2neg(float x) { return 1.f / pow2(x); }
  inline uint32_t nextRand() { uint32_t r = rng_; r ^= r << 13; r ^= r >> 17; r ^= r << 5; rng_ = r; return r; }
  inline float rand01() { return (nextRand() >> 8) * (1.f / 16777216.f); }
  float *bL_ = nullptr, *bR_ = nullptr; uint32_t w_ = 0, rng_;
  float rate_, depth_, fb_, manual_, stereo_, age_, mix_; uint8_t mode_, shape_; bool neg_;
  float ph_, bph_[3], fbL_, fbR_, lpL_, lpR_, env_, rv_, rt_, dL_, dR_, dlL_ = 0.f, dlR_ = 0.f; int32_t rn_;
};
