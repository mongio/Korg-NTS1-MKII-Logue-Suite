#pragma once
/*
 *  GESTO v0.1 - registra il movimento del knob B e lo ripete in loop, a tempo, come modulazione
 *  (filtro risonante, panorama, intonazione, volume, filtro+panorama). Nessuna dipendenza da libm.
 *  La curva e' indicizzata in battiti (64 punti per battito): cambiando il tempo del NTS-1 resta a tempo.
 */
#include <stdint.h>
class Gesto {
 public:
  enum { P_AMOUNT = 0, P_GESTO, P_DEST, P_ACTION, P_LENGTH, P_DIR, P_SPEED, P_SMOOTH, P_WEAR, P_RESO, P_NUM };
  enum { D_FILTER = 0, D_PAN, D_PITCH, D_VOLUME, D_FILTPAN };
  enum { A_LIVE = 0, A_REC, A_PLAY };
  enum { R_IDLE = 0, R_ARMED, R_REC, R_PLAY };
  enum { PPB = 64, MAXBEATS = 32, CN = PPB * MAXBEATS, PN = 4096, PM = PN - 1 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kFloats = CN + 2 * PN;

  void init(float *mem) {
    curve_ = mem; pbL_ = mem + CN; pbR_ = mem + CN + PN;
    amount_ = 0.7f; knob_ = 0.5f; dest_ = D_FILTER; action_ = A_LIVE; lenIdx_ = 1; dir_ = 0; spIdx_ = 2; smooth_ = 0.3f; wear_ = 0.f; reso_ = 0.4f;
    bpm_ = 120.f; clear();
  }
  void clear() {
    for (uint32_t i = 0; i < kFloats; ++i) curve_[i] = 0.f;
    for (int i = 0; i < CN; ++i) curve_[i] = 0.5f;
    state_ = R_IDLE; have_ = false; beat_ = 0.f; recLen_ = 8; armKnob_ = knob_; m_ = 0.5f; ms_ = 0.5f; ping_ = 1;
    lp1L_ = lp2L_ = lp1R_ = lp2R_ = 0.f; bp1L_ = bp1R_ = 0.f; pw_ = 0; pph_ = 0.f; rng_ = 0x6E570A11u; tick_ = 0;
    gL_ = gR_ = 1.f; recIdx_ = 0;
  }
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_AMOUNT: amount_ = v * (1.f / 1023.f); break;
      case P_GESTO:  knob_ = v * (1.f / 1023.f); break;
      case P_DEST:   dest_ = (uint8_t)clampi(v, 0, 4); break;
      case P_ACTION: {
        const uint8_t a = (uint8_t)clampi(v, 0, 2);
        if (a != action_) {
          action_ = a;
          if (a == A_REC) { state_ = R_ARMED; armKnob_ = knob_; }
          else if (a == A_PLAY) state_ = have_ ? R_PLAY : R_IDLE;
          else state_ = R_IDLE;
        }
      } break;
      case P_LENGTH: lenIdx_ = (uint8_t)clampi(v, 0, 3); break;
      case P_DIR:    dir_ = (uint8_t)clampi(v, 0, 2); break;
      case P_SPEED:  spIdx_ = (uint8_t)clampi(v, 0, 4); break;
      case P_SMOOTH: smooth_ = v * 0.01f; break;
      case P_WEAR:   wear_ = v * v * 0.0001f; break;              // corsa progressiva: valori bassi consumano piano
      case P_RESO:   reso_ = v * 0.01f; break;
      default: break;
    }
  }
  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *dests[] = {"FILTER", "PAN", "PITCH", "VOLUME", "FILTPAN"};
    static const char *acts[] = {"LIVE", "REC", "PLAY"};
    static const char *lens[] = {"1BAR", "2BAR", "4BAR", "8BAR"};
    static const char *dirs[] = {"FWD", "REV", "PINGPONG"};
    static const char *sps[] = {"0.25X", "0.5X", "1X", "2X", "4X"};
    switch (id) {
      case P_DEST:   if (v >= 0 && v < 5) return dests[v]; break;
      case P_ACTION: if (v >= 0 && v < 3) return acts[v]; break;
      case P_LENGTH: if (v >= 0 && v < 4) return lens[v]; break;
      case P_DIR:    if (v >= 0 && v < 3) return dirs[v]; break;
      case P_SPEED:  if (v >= 0 && v < 5) return sps[v]; break;
      default: break;
    }
    return nullptr;
  }
  void setTempo(float bpm) { bpm_ = bpm < 30.f ? 30.f : (bpm > 300.f ? 300.f : bpm); }
  int32_t actionNow() const { return (int32_t)action_; }      // lo stato vero (dopo la registrazione: PLAY)
  bool recording() const { return state_ == R_REC; }
  bool playing() const { return state_ == R_PLAY; }

  void process(const float *in, float *out, uint32_t frames) {
    for (uint32_t n = 0; n < frames; ++n) {
      if ((tick_++ & 15u) == 0u) control(16);
      // la curva arriva morbida (SMOOTH) e un po' piu' veloce del controllo
      ms_ += (m_ - ms_) * smc_;
      const float m = ms_;
      float l = in[2 * n], r = in[2 * n + 1];
      switch (dest_) {
        case D_PAN: pan(l, r, m); break;
        case D_PITCH: pitch(l, r, m); break;
        case D_VOLUME: { const float g = 1.f - amount_ * (1.f - m); l *= g; r *= g; } break;
        case D_FILTPAN: filter(l, r, m); pan(l, r, m); break;
        default: filter(l, r, m); break;
      }
      out[2 * n] = lim(l); out[2 * n + 1] = lim(r);
    }
  }

 private:
  // ---------- registrazione e riproduzione del gesto ----------
  void control(int ns) {
    static const float spd[5] = {0.25f, 0.5f, 1.f, 2.f, 4.f};
    static const int lens[4] = {4, 8, 16, 32};                       // battiti: 1, 2, 4, 8 misure in 4/4
    const float db = bpm_ / 60.f * (float)ns / kSR;                   // battiti trascorsi
    // SMOOTH: da ~1 ms a ~300 ms
    { const float tc = 0.001f * pow2(smooth_ * 8.2f); smc_ = 1.f / (1.f + tc * kSR); }
    if (state_ == R_ARMED) {                                         // parte al primo movimento del knob B
      const float d = knob_ - armKnob_;
      if (d > 0.02f || d < -0.02f) { state_ = R_REC; beat_ = 0.f; recLen_ = lens[lenIdx_]; recIdx_ = 0; }
      m_ = knob_;
    } else if (state_ == R_REC) {
      beat_ += db;
      const int target = (int)(beat_ * PPB);
      while (recIdx_ <= target && recIdx_ < recLen_ * PPB) curve_[recIdx_++] = knob_;
      m_ = knob_;
      if (beat_ >= (float)recLen_) { have_ = true; state_ = R_PLAY; action_ = A_PLAY; beat_ = 0.f; ping_ = 1; }
    } else if (state_ == R_PLAY) {
      const float L = (float)recLen_;
      beat_ += db * spd[spIdx_];
      float pos;
      if (dir_ == 2) {                                               // avanti e indietro
        while (beat_ >= L) { beat_ -= L; ping_ = -ping_; }
        pos = ping_ > 0 ? beat_ : L - beat_;
      } else {
        while (beat_ >= L) beat_ -= L;
        pos = dir_ == 1 ? L - beat_ : beat_;
      }
      const float fi = pos * PPB; int i0 = (int)fi; const float fr = fi - (float)i0;
      const int N = recLen_ * PPB; if (i0 >= N) i0 = N - 1; const int i1 = (i0 + 1 < N) ? i0 + 1 : 0;
      m_ = curve_[i0] + (curve_[i1] - curve_[i0]) * fr;
      if (wear_ > 0.001f) {                                          // WEAR: il gesto si consuma a ogni passaggio
        const int ip = i0 > 0 ? i0 - 1 : N - 1;
        const float avg = 0.5f * (curve_[ip] + curve_[i1]);
        // perdita di dettaglio + tremolio + lenta deriva verso il centro (copia di una copia)
        float c = curve_[i0] + wear_ * (0.06f * (avg - curve_[i0]) + 0.012f * (rand01() - 0.5f) + 0.0015f * (0.5f - curve_[i0]));
        curve_[i0] = c < 0.f ? 0.f : (c > 1.f ? 1.f : c);
      }
    } else {
      m_ = knob_;                                                    // LIVE (o nessun gesto registrato)
    }
    // coefficienti del filtro (FILTER, FILTPAN): taglio esponenziale 120 Hz..12 kHz
    if (dest_ == D_FILTER || dest_ == D_FILTPAN) {
      const float e = 6.64f * (1.f - amount_ * (1.f - ms_));
      const float fc = 120.f * pow2(e);
      float g = 3.14159265f * fc / kSR; if (g > 0.95f) g = 0.95f;
      fg_ = g / (1.f + g); fq_ = 1.6f - 1.55f * reso_;             // smorzamento: 1,6 (morbido) .. 0,05 (risonante)
    }
  }
  // filtro a variabile di stato (passa basso risonante), 2 canali
  inline void filter(float &l, float &r, float m) {
    (void)m;
    const float g = fg_, k = fq_;
    { const float hp = (l - (k + g) * bp1L_ - lp1L_) / (1.f + g * (k + g)); const float bp = g * hp + bp1L_; bp1L_ = g * hp + bp; const float lp = g * bp + lp1L_; lp1L_ = g * bp + lp; l = soft(lp); }
    { const float hp = (r - (k + g) * bp1R_ - lp1R_) / (1.f + g * (k + g)); const float bp = g * hp + bp1R_; bp1R_ = g * hp + bp; const float lp = g * bp + lp1R_; lp1R_ = g * bp + lp; r = soft(lp); }
  }
  inline void pan(float &l, float &r, float m) {
    const float p = (m - 0.5f) * 2.f * amount_;                     // -1 sinistra .. +1 destra
    const float tl = p > 0.f ? 1.f - p : 1.f, tr = p < 0.f ? 1.f + p : 1.f;
    gL_ += (tl - gL_) * 0.01f; gR_ += (tr - gR_) * 0.01f;
    const float mono = 0.5f * (l + r);
    l = (l * (1.f - 0.5f * amount_) + mono * 0.5f * amount_) * gL_ * 1.2f;
    r = (r * (1.f - 0.5f * amount_) + mono * 0.5f * amount_) * gR_ * 1.2f;
  }
  // PITCH: trasposizione a due testine (fino a +/- 12 semitoni con AMOUNT al massimo)
  inline void pitch(float &l, float &r, float m) {
    pbL_[pw_] = l; pbR_[pw_] = r;
    const float semi = (m - 0.5f) * 24.f * amount_;
    const float ratio = pow2(semi * (1.f / 12.f));
    const float W = 1600.f;                                         // finestra ~33 ms
    pph_ += (1.f - ratio) / W; if (pph_ >= 1.f) pph_ -= 1.f; if (pph_ < 0.f) pph_ += 1.f;
    float ol = 0.f, orr = 0.f;
    for (int h = 0; h < 2; ++h) {
      float q = pph_ + 0.5f * h; if (q >= 1.f) q -= 1.f;
      const float d = 8.f + q * W;
      float rp = (float)pw_ - d; if (rp < 0.f) rp += (float)PN; const int32_t ri = (int32_t)rp; const float fr = rp - (float)ri;
      const float u = q * (1.f - q); const float wgt = 16.f * u / (5.f - 4.f * u);   // ~sin(pi q)
      const float w2 = wgt * wgt;
      ol += (pbL_[ri & PM] + (pbL_[(ri + 1) & PM] - pbL_[ri & PM]) * fr) * w2;
      orr += (pbR_[ri & PM] + (pbR_[(ri + 1) & PM] - pbR_[ri & PM]) * fr) * w2;
    }
    pw_ = (pw_ + 1) & PM;
    const float wet = amount_ > 0.001f ? 1.f : 0.f;
    l = l * (1.f - wet) + ol * wet; r = r * (1.f - wet) + orr * wet;
  }
  static inline int clampi(int32_t v, int a, int b) { return v < a ? a : (v > b ? b : (int)v); }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }
  static inline float lim(float y) { const float a = y < 0.f ? -y : y; if (a <= 0.8f) return y; const float e = (a - 0.8f) * 5.f; const float l = 0.8f + 0.2f * e / (1.f + e); return y < 0.f ? -l : l; }
  static inline float pow2(float x) { const bool ng = x < 0.f; if (ng) x = -x; const int32_t ip = (int32_t)x; const float f = x - ip;
    const float r = (float)(1u << (ip > 30 ? 30 : ip)) * (1.f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * 0.00961813f)))); return ng ? 1.f / r : r; }
  inline float rand01() { rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5; return (rng_ >> 8) * (1.f / 16777216.f); }

  float *curve_ = nullptr, *pbL_ = nullptr, *pbR_ = nullptr;
  float amount_, knob_, smooth_, wear_, reso_, bpm_, beat_, armKnob_, m_, ms_, smc_ = 0.01f;
  uint8_t dest_, action_, lenIdx_, dir_, spIdx_, state_; bool have_; int recLen_, recIdx_, ping_;
  float lp1L_, lp2L_, lp1R_, lp2R_, bp1L_, bp1R_, fg_ = 0.5f, fq_ = 1.f, gL_, gR_, pph_; uint32_t pw_, rng_, tick_;
};
