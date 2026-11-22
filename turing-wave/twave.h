#pragma once
/*
 *  File: twave.h  (TURING WAVE v1.0)
 *  Il registro scorre LENGTH volte per ciclo: i suoi bit, letti come DAC,
 *  formano la forma d'onda. LOCK decide quanto l'onda muta mentre suona.
 *  Salti tra i passi corretti con PolyBLEP (1 campione di ritardo).
 *  Nessuna dipendenza da libm.
 */
#include <stdint.h>
#include "unit_osc.h"

class TWaveOsc {
 public:
  enum { P_LOCK = 0, P_TONE, P_LENGTH, P_READ, P_MODE, P_NOTE, P_BURST, P_SUB, P_DRIVE, P_NUM };
  enum { R_1BIT = 0, R_4BIT, R_8BIT, R_SMOOTH };
  static constexpr float kSR = 48000.f;

  void init() {
    lock_ = 0.9f; tone_ = 0.7f; len_ = 16; read_ = R_8BIT; mode_ = 0; note_new_ = false;
    burst_ = 0.f; sub_ = 0.f; drive_ = 0.f; lfo_ = 0.f;
    rng_ = 0x9E3779B9u;
    reg_ = nextRand();
    acc_ = 0.f; cur_ = prev_val_ = dacValue(); y_prev_ = 0.f;
    sub_ph_ = 0.f; sub_sign_ = 1.f; lp1_ = lp2_ = 0.f; hp_x_ = hp_y_ = 0.f; env_ = 0.f;
  }

  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_LOCK:   lock_ = v * (1.f / 1023.f); break;
      case P_TONE:   tone_ = v * (1.f / 1023.f); break;
      case P_LENGTH: len_ = (uint8_t)(v + 2); break;
      case P_READ:   read_ = (uint8_t)v; break;
      case P_MODE:   mode_ = (uint8_t)v; break;
      case P_NOTE:   note_new_ = (v != 0); break;
      case P_BURST:  burst_ = v * 0.01f; break;
      case P_SUB:    sub_ = v * 0.01f; break;
      case P_DRIVE:  drive_ = v * 0.01f; break;
      default: break;
    }
  }

  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *lens[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17",
                                 "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32"};
    static const char *reads[] = {"1BIT", "4BIT", "8BIT", "SMOOTH"};
    static const char *modes[] = {"TURING", "LFSR"};
    static const char *notes[] = {"KEEP", "NEW"};
    switch (id) {
      case P_LENGTH: if (v >= 0 && v < 31) return lens[v]; break;
      case P_READ:   if (v >= 0 && v < 4) return reads[v]; break;
      case P_MODE:   if (v >= 0 && v < 2) return modes[v]; break;
      case P_NOTE:   if (v >= 0 && v < 2) return notes[v]; break;
      default: break;
    }
    return nullptr;
  }

  void setLfo(float l) { lfo_ = l; }
  void noteOn() {
    if (note_new_) { reg_ = nextRand(); if (!reg_) reg_ = 1; }
    if (burst_ > 0.f) env_ = 1.f;
  }

  void process(float base_note, float *out, uint32_t frames) {
    float n = base_note; if (n < 0.f) n = 0.f; if (n > 151.f) n = 151.f;
    const uint8_t ni = (uint8_t)n;
    const float w = osc_w0f_for_note(ni, (uint8_t)((n - ni) * 255.f));   // f0 / fs
    const float L = (float)len_;
    const float sw = w * L;                                                // passi per campione

    // probabilita' di mutazione per passo (curva cubica, simmetrica sul centro)
    float k = lock_ + lfo_ * 0.5f;
    k = k < 0.f ? 0.f : (k > 1.f ? 1.f : k);
    float p;
    if (k >= 0.98f) p = 0.f;
    else if (k <= 0.02f) p = 1.f;
    else if (k >= 0.5f) { const float u = (1.f - k) * 2.f; p = u * u * u * 0.5f; }
    else { const float u = k * 2.f; p = 1.f - u * u * u * 0.5f; }

    // filtro TONE: taglio = f0 x (1..128), 2 poli, a inseguimento della nota
    const float mult = pow2(tone_ * 7.f);
    float fc = w * mult;                    // normalizzato
    const bool lp_on = tone_ < 0.995f && fc < 0.45f;
    if (fc > 0.45f) fc = 0.45f;
    const float x = 6.2831853f * fc;
    const float c = x / (1.f + x);

    const float env_dec = 1.f - 1.f / (0.15f * kSR);   // ~150 ms
    const float drv = 1.f + drive_ * 6.f;
    const float sub_w = w * 0.5f;
    const float sub_norm = 1.f / (1.f + sub_ * 0.8f);
    const float drv_makeup = (1.f + drv) / drv;

    for (uint32_t i = 0; i < frames; ++i) {
      // mutazione extra al note-on (BURST)
      float pe = p;
      if (env_ > 0.0001f) { const float b = env_ * burst_ * 0.5f; if (b > pe && pe < 0.5f) pe = b; env_ *= env_dec; }

      float y = cur_;       // valore naive di questo campione
      float corr_now = 0.f, corr_prev = 0.f;
      acc_ += sw;
      int guard = 0;
      while (acc_ >= 1.f && guard < 16) {
        acc_ -= 1.f; ++guard;
        prev_val_ = cur_;
        stepRegister(pe);
        cur_ = dacValue();
        if (read_ != R_SMOOTH) {
          const float h = cur_ - prev_val_;
          const float d = sw > 1.f ? 0.f : acc_ / sw;   // tempo dal salto, in campioni (0..1)
          corr_now  += -0.5f * h * (1.f - d) * (1.f - d);
          corr_prev +=  0.5f * h * d * d;
        }
      }
      if (guard == 16) acc_ = 0.f;
      if (read_ == R_SMOOTH) {
        const float fr = acc_ < 1.f ? acc_ : 1.f;
        y = prev_val_ + (cur_ - prev_val_) * fr;
      } else {
        y = cur_ + corr_now;
      }

      // sub-oscillatore quadro un'ottava sotto (PolyBLEP)
      float s = 0.f;
      if (sub_ > 0.f) {
        sub_ph_ += sub_w;
        if (sub_ph_ >= 1.f) sub_ph_ -= 1.f;
        float p2 = sub_ph_ + 0.5f; if (p2 >= 1.f) p2 -= 1.f;
        s = ((sub_ph_ < 0.5f ? 1.f : -1.f) + blep(sub_ph_, sub_w) - blep(p2, sub_w)) * sub_;
      }

      // uscita ritardata di 1 campione per applicare la correzione "prima del salto"
      float o = y_prev_ + corr_prev;
      y_prev_ = y;
      if (read_ == R_1BIT) o *= 0.7f;
      o = (o + s) * sub_norm;

      // TONE
      if (lp_on) { lp1_ += (o - lp1_) * c; lp2_ += (lp1_ - lp2_) * c; o = lp2_; }

      // DRIVE (saturazione morbida, livello compensato)
      if (drive_ > 0.f) { const float z = o * drv; o = z / (1.f + (z < 0.f ? -z : z)) * drv_makeup; }

      // blocco DC (~20 Hz)
      const float hy = o - hp_x_ + 0.9974f * hp_y_;
      hp_x_ = o; hp_y_ = hy;

      float r = hy * 0.75f;
      // limitatore morbido sopra 0.8 (asintoto a 1.0)
      const float a = r < 0.f ? -r : r;
      if (a > 0.8f) { const float e = (a - 0.8f) * 5.f; const float lim = 0.8f + 0.2f * e / (1.f + e); r = r < 0.f ? -lim : lim; }
      out[i] = r;
    }
  }

 private:
  static inline float blep(float t, float dt) {
    if (t < dt) { const float x = t / dt; return x + x - x * x - 1.f; }
    if (t > 1.f - dt) { const float x = (t - 1.f) / dt; return x * x + x + x + 1.f; }
    return 0.f;
  }
  // 2^x per x in [0,8], senza libm
  static inline float pow2(float x) {
    const int32_t ip = (int32_t)x;
    const float f = x - ip;
    const float poly = 1.f + f * (0.6931472f + f * (0.2402265f + f * 0.0555041f));
    return (float)(1u << ip) * poly;
  }
  inline uint32_t nextRand() { uint32_t x = rng_; x ^= x << 13; x ^= x >> 17; x ^= x << 5; rng_ = x; return x; }

  inline void stepRegister(float p) {
    const uint8_t Lb = len_;
    uint32_t bit = (reg_ >> (Lb - 1)) & 1u;
    if (mode_ == 1) bit ^= (reg_ >> (Lb - 2)) & 1u;   // LFSR: retroazione XOR
    if (p > 0.f && (nextRand() >> 8) * (1.f / 16777216.f) < p) bit ^= 1u;
    reg_ = (reg_ << 1) | bit;
    if (mode_ == 1 && (reg_ & lenMask()) == 0) reg_ |= 1u;  // evita il blocco a zero dell'LFSR
  }
  inline uint32_t lenMask() const { return len_ >= 32 ? 0xFFFFFFFFu : ((1u << len_) - 1u); }

  inline float dacValue() const {
    switch (read_) {
      case R_1BIT: return (reg_ & 1u) ? 1.f : -1.f;
      case R_4BIT: return (float)(reg_ & 0xFu) * (2.f / 15.f) - 1.f;
      default:     return (float)(reg_ & 0xFFu) * (2.f / 255.f) - 1.f;
    }
  }

  float lock_, tone_, burst_, sub_, drive_, lfo_;
  uint8_t len_, read_, mode_;
  bool note_new_;
  uint32_t rng_, reg_;
  float acc_, cur_, prev_val_, y_prev_, sub_ph_, sub_sign_, lp1_, lp2_, hp_x_, hp_y_, env_;
};
