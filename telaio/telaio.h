#pragma once
/*
 *  TELAIO v0.1 - la macchina di Turing applicata alla modulazione: un registro di bit che gira a tempo genera
 *  modulazioni a gradini (o scivolate) su filtro, panorama, intonazione (quantizzata su una scala), volume.
 *  LOCK: a destra il disegno si ripete, a sinistra muta sempre. Parte sonora presa da GESTO. Nessuna dipendenza da libm.
 */
#include <stdint.h>
class Telaio {
 public:
  enum { P_AMOUNT = 0, P_LOCK, P_DEST, P_RATE, P_LENGTH, P_SLIDE, P_SCALE, P_OFFSET, P_RESO, P_MIX, P_NUM };
  enum { D_FILTER = 0, D_PAN, D_PITCH, D_VOLUME, D_FILTPAN };
  enum { PN = 4096, PM = PN - 1 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kFloats = 2 * PN;

  void init(float *mem) {
    pbL_ = mem; pbR_ = mem + PN;
    amount_ = 0.6f; lock_ = 0.75f; dest_ = D_FILTER; rate_ = 3; len_ = 8; slide_ = 0.2f; scale_ = 2; offset_ = 0.5f; reso_ = 0.4f; mix_ = 1.f;
    bpm_ = 120.f; clear();
  }
  void clear() {
    for (uint32_t i = 0; i < kFloats; ++i) pbL_[i] = 0.f;
    rng_ = 0x7E1A1005u; reg_ = 0xB5C3u; beat_ = 0.f; lastStep_ = -1; ticks_ = 0; tickAge_ = 1 << 30; target_ = 0.5f; m_ = 0.5f;
    lp1L_ = lp1R_ = bp1L_ = bp1R_ = 0.f; pw_ = 0; pph_ = 0.f; gL_ = gR_ = 1.f; tick_ = 0; ratio_ = 1.f; semiT_ = semi_ = 0.f; fg_ = 0.5f; fq_ = 1.f;
    step();   // primo valore
  }
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_AMOUNT: amount_ = v * (1.f / 1023.f); break;
      case P_LOCK:   lock_ = v * (1.f / 1023.f); break;
      case P_DEST:   dest_ = (uint8_t)clampi(v, 0, 4); break;
      case P_RATE:   rate_ = (uint8_t)clampi(v, 0, 5); break;
      case P_LENGTH: len_ = 2 + clampi(v, 0, 14); break;
      case P_SLIDE:  slide_ = v * 0.01f; break;
      case P_SCALE:  scale_ = (uint8_t)clampi(v, 0, 4); break;
      case P_OFFSET: offset_ = v * 0.01f; break;
      case P_RESO:   reso_ = v * 0.01f; break;
      case P_MIX:    mix_ = v * 0.01f; break;
      default: break;
    }
  }
  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *dests[] = {"FILTER", "PAN", "PITCH", "VOLUME", "FILTPAN"};
    static const char *rates[] = {"1BAR", "2BEAT", "1BEAT", "8TH", "16TH", "32ND"};
    static const char *lens[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
    static const char *scales[] = {"CHROM", "MAJOR", "MINOR", "PENTA", "OCTAVE"};
    switch (id) {
      case P_DEST:   if (v >= 0 && v < 5) return dests[v]; break;
      case P_RATE:   if (v >= 0 && v < 6) return rates[v]; break;
      case P_LENGTH: if (v >= 0 && v < 15) return lens[v]; break;
      case P_SCALE:  if (v >= 0 && v < 5) return scales[v]; break;
      default: break;
    }
    return nullptr;
  }
  void setTempo(float bpm) { bpm_ = bpm < 30.f ? 30.f : (bpm > 300.f ? 300.f : bpm); }
  void tick4ppqn() { ++ticks_; tickAge_ = 0; }                      // 4 impulsi per quarto (sedicesimi)

  void process(const float *in, float *out, uint32_t frames) {
    for (uint32_t n = 0; n < frames; ++n) {
      if ((tick_++ & 15u) == 0u) control(16);
      m_ += (target_ - m_) * sc_;                                    // SLIDE (minimo ~1 ms contro i clic)
      semi_ += (semiT_ - semi_) * sc_;
      float l = in[2 * n], r = in[2 * n + 1];
      const float dl = l, dr = r;
      switch (dest_) {
        case D_PAN: pan(l, r, m_); break;
        case D_PITCH: ratio_ = pow2(semi_ * (1.f / 12.f)); pitch(l, r, m_); break;
        case D_VOLUME: { const float g = 1.f - amount_ * (1.f - m_); l *= g; r *= g; } break;
        case D_FILTPAN: filter(l, r, m_); pan(l, r, m_); break;
        default: filter(l, r, m_); break;
      }
      out[2 * n] = lim(dl * (1.f - mix_) + l * mix_);
      out[2 * n + 1] = lim(dr * (1.f - mix_) + r * mix_);
    }
  }

 private:
  // ---------- l'orologio e la macchina di Turing ----------
  void control(int ns) {
    static const float div[6] = {4.f, 2.f, 1.f, 0.5f, 0.25f, 0.125f};   // battiti per passo
    tickAge_ += ns;
    long st;
    if (tickAge_ < (int32_t)kSR) {                                  // clock del NTS-1 presente: passi agganciati agli impulsi
      const float tpb = 4.f;                                        // impulsi per battito
      const float perStep = div[rate_] * tpb;
      if (perStep >= 1.f) st = (long)((float)ticks_ / perStep);
      else { beat_ += bpm_ / 60.f * (float)ns / kSR; st = (long)(beat_ / div[rate_]); }
    } else {                                                        // nessun clock: tempo impostato
      beat_ += bpm_ / 60.f * (float)ns / kSR;
      st = (long)(beat_ / div[rate_]);
    }
    if (st != lastStep_) { lastStep_ = st; step(); }
    // SLIDE: costante di tempo proporzionale alla durata del passo
    const float stepSec = div[rate_] * 60.f / bpm_;
    const float tc = 0.0012f + slide_ * slide_ * stepSec * 0.45f;
    sc_ = 1.f / (1.f + tc * kSR);
    // filtro (FILTER, FILTPAN): esponenziale 120 Hz .. 12 kHz attorno a OFFSET
    if (dest_ == D_FILTER || dest_ == D_FILTPAN) {
      float e = 6.64f * (offset_ + (m_ - 0.5f) * amount_ * 1.2f); e = e < 0.f ? 0.f : (e > 6.64f ? 6.64f : e);
      float g = 3.14159265f * (120.f * pow2(e)) / kSR; if (g > 0.95f) g = 0.95f;
      fg_ = g / (1.f + g); fq_ = 1.6f - 1.55f * reso_;
    }
  }
  void step() {
    const uint32_t mask = (1u << len_) - 1u;
    uint32_t bit = (reg_ >> (len_ - 1)) & 1u;                       // il bit che esce rientra dall'altra parte...
    const float flip = 0.5f * (1.f - lock_) * (1.f - lock_);       // LOCK 1 = mai (si ripete), 0,5 = 12,5%, 0 = 50% (del tutto casuale)
    if (rand01() < flip) bit ^= 1u;                                 // ...e con probabilita' (1-LOCK) viene invertito
    reg_ = ((reg_ << 1) | bit) & mask;
    // valore del passo: gli ultimi 8 bit (ripetuti se il registro e' piu' corto)
    uint32_t v = 0; for (int k = 0; k < 8; ++k) v = (v << 1) | ((reg_ >> (k % len_)) & 1u);
    target_ = (float)v * (1.f / 255.f);
    // PITCH: semitoni quantizzati sulla scala, attorno a OFFSET (+/- 12 semitoni con AMOUNT al massimo)
    static const int8_t SC[5][12] = {
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {0, 0, 2, 2, 4, 5, 5, 7, 7, 9, 9, 11}, {0, 0, 2, 3, 3, 5, 5, 7, 8, 8, 10, 10},
      {0, 0, 2, 2, 4, 4, 4, 7, 7, 9, 9, 9}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    const float raw = (target_ - 0.5f) * 24.f * amount_ + (offset_ - 0.5f) * 24.f;
    int s = (int)(raw + (raw >= 0.f ? 0.5f : -0.5f)); const int oc = (s >= 0 ? s / 12 : -((-s + 11) / 12)); int pc = s - oc * 12;
    semiT_ = (float)(oc * 12 + SC[scale_][pc]);
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
    (void)m; const float ratio = ratio_;
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
    l = ol; r = orr;
  }
  static inline int clampi(int32_t v, int a, int b) { return v < a ? a : (v > b ? b : (int)v); }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }
  static inline float lim(float y) { const float a = y < 0.f ? -y : y; if (a <= 0.8f) return y; const float e = (a - 0.8f) * 5.f; const float l = 0.8f + 0.2f * e / (1.f + e); return y < 0.f ? -l : l; }
  static inline float pow2(float x) { const bool ng = x < 0.f; if (ng) x = -x; const int32_t ip = (int32_t)x; const float f = x - ip;
    const float r = (float)(1u << (ip > 30 ? 30 : ip)) * (1.f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * 0.00961813f)))); return ng ? 1.f / r : r; }
  inline float rand01() { rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5; return (rng_ >> 8) * (1.f / 16777216.f); }

  float *pbL_ = nullptr, *pbR_ = nullptr;
  float amount_, lock_, slide_, offset_, reso_, mix_, bpm_, beat_, target_, m_, sc_ = 0.02f, ratio_, semiT_, semi_;
  uint8_t dest_, rate_, scale_; int len_; uint32_t rng_, reg_, ticks_, pw_, tick_; int32_t tickAge_; long lastStep_;
  float lp1L_, lp1R_, bp1L_, bp1R_, fg_, fq_, gL_, gR_, pph_;
};
