#pragma once
/*
 *  ARMONIA v0.1 - armonizzatore denso: due voci trasposte (ciascuna raddoppiata da una copia scordata), quantizzate su
 *  una scala, con shimmer (le voci rientrano in se stesse), grana, frammenti al contrario, deriva e stereo largo.
 *  Trasposto a due testine con finestre sin^2; quando una testina riparte (peso zero) sceglie, nei ~5 ms successivi,
 *  il punto piu' somigliante a quello che suona l'altra testina (meno "phasing" sui suoni tenuti).
 *  Nessuna dipendenza da libm.
 */
#include <stdint.h>
static const float kWin[257] = {0.0000000f,0.0001506f,0.0006023f,0.0013548f,0.0024076f,0.0037602f,0.0054117f,0.0073612f,0.0096074f,0.0121489f,0.0149844f,0.0181120f,0.0215298f,0.0252359f,0.0292280f,0.0335036f,0.0380602f,0.0428951f,0.0480054f,0.0533878f,0.0590394f,0.0649565f,0.0711357f,0.0775732f,0.0842652f,0.0912076f,0.0983962f,0.1058268f,0.1134948f,0.1213956f,0.1295244f,0.1378765f,0.1464466f,0.1552297f,0.1642205f,0.1734136f,0.1828034f,0.1923842f,0.2021503f,0.2120959f,0.2222149f,0.2325012f,0.2429486f,0.2535509f,0.2643016f,0.2751943f,0.2862225f,0.2973793f,0.3086583f,0.3200525f,0.3315551f,0.3431591f,0.3548577f,0.3666436f,0.3785099f,0.3904494f,0.4024548f,0.4145191f,0.4266348f,0.4387947f,0.4509914f,0.4632177f,0.4754662f,0.4877294f,0.5000000f,0.5122706f,0.5245338f,0.5367823f,0.5490086f,0.5612053f,0.5733652f,0.5854809f,0.5975452f,0.6095506f,0.6214901f,0.6333564f,0.6451423f,0.6568409f,0.6684449f,0.6799475f,0.6913417f,0.7026207f,0.7137775f,0.7248057f,0.7356984f,0.7464491f,0.7570514f,0.7674988f,0.7777851f,0.7879041f,0.7978497f,0.8076158f,0.8171966f,0.8265864f,0.8357795f,0.8447703f,0.8535534f,0.8621235f,0.8704756f,0.8786044f,0.8865052f,0.8941732f,0.9016038f,0.9087924f,0.9157348f,0.9224268f,0.9288643f,0.9350435f,0.9409606f,0.9466122f,0.9519946f,0.9571049f,0.9619398f,0.9664964f,0.9707720f,0.9747641f,0.9784702f,0.9818880f,0.9850156f,0.9878511f,0.9903926f,0.9926388f,0.9945883f,0.9962398f,0.9975924f,0.9986452f,0.9993977f,0.9998494f,1.0000000f,0.9998494f,0.9993977f,0.9986452f,0.9975924f,0.9962398f,0.9945883f,0.9926388f,0.9903926f,0.9878511f,0.9850156f,0.9818880f,0.9784702f,0.9747641f,0.9707720f,0.9664964f,0.9619398f,0.9571049f,0.9519946f,0.9466122f,0.9409606f,0.9350435f,0.9288643f,0.9224268f,0.9157348f,0.9087924f,0.9016038f,0.8941732f,0.8865052f,0.8786044f,0.8704756f,0.8621235f,0.8535534f,0.8447703f,0.8357795f,0.8265864f,0.8171966f,0.8076158f,0.7978497f,0.7879041f,0.7777851f,0.7674988f,0.7570514f,0.7464491f,0.7356984f,0.7248057f,0.7137775f,0.7026207f,0.6913417f,0.6799475f,0.6684449f,0.6568409f,0.6451423f,0.6333564f,0.6214901f,0.6095506f,0.5975452f,0.5854809f,0.5733652f,0.5612053f,0.5490086f,0.5367823f,0.5245338f,0.5122706f,0.5000000f,0.4877294f,0.4754662f,0.4632177f,0.4509914f,0.4387947f,0.4266348f,0.4145191f,0.4024548f,0.3904494f,0.3785099f,0.3666436f,0.3548577f,0.3431591f,0.3315551f,0.3200525f,0.3086583f,0.2973793f,0.2862225f,0.2751943f,0.2643016f,0.2535509f,0.2429486f,0.2325012f,0.2222149f,0.2120959f,0.2021503f,0.1923842f,0.1828034f,0.1734136f,0.1642205f,0.1552297f,0.1464466f,0.1378765f,0.1295244f,0.1213956f,0.1134948f,0.1058268f,0.0983962f,0.0912076f,0.0842652f,0.0775732f,0.0711357f,0.0649565f,0.0590394f,0.0533878f,0.0480054f,0.0428951f,0.0380602f,0.0335036f,0.0292280f,0.0252359f,0.0215298f,0.0181120f,0.0149844f,0.0121489f,0.0096074f,0.0073612f,0.0054117f,0.0037602f,0.0024076f,0.0013548f,0.0006023f,0.0001506f,0.0000000f};   // sin^2(pi q)
class Armonia {
 public:
  enum { P_INTERV = 0, P_MIX, P_VOICE2, P_SCALE, P_DETUNE, P_SHIMMER, P_GRAIN, P_REVERSE, P_DRIFT, P_STEREO, P_NUM };
  enum { N = 8192, MASK = N - 1, NS = 8, SEARCH = 240, CL = 48, NCAND = 24 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kFloats = 2 * N;

  void init(float *mem) {
    b_[0] = mem; b_[1] = mem + N;
    interv_ = 31; mix_ = 0.6f; v2_ = 1 + 12; scale_ = 0; detune_ = 0.45f; shimmer_ = 0.25f; grain_ = 0.5f; rev_ = 0; drift_ = 0.25f; stereo_ = 0.8f;
    clear();
  }
  void clear() {
    for (uint32_t i = 0; i < kFloats; ++i) b_[0][i] = 0.f;
    w_ = 0; tick_ = 0; rng_ = 0x41A30715u; fbL_ = fbR_ = 0.f; fbs_[0] = fbs_[1] = 0.f;
    for (int i = 0; i < NS; ++i) { ph_[i] = 0.13f * i; off_[i][0] = off_[i][1] = 0.f; rat_[i] = 1.f; dr_[i] = drT_[i] = 0.f; drCnt_[i] = 0; }
    updateRatios();
  }
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_INTERV:  interv_ = clampi(v, 0, 48); break;
      case P_MIX:     mix_ = v * (1.f / 1023.f); break;
      case P_VOICE2:  v2_ = clampi(v, 0, 49); break;
      case P_SCALE:   scale_ = (uint8_t)clampi(v, 0, 4); break;
      case P_DETUNE:  detune_ = v * 0.01f; break;
      case P_SHIMMER: shimmer_ = v * 0.01f; break;
      case P_GRAIN:   grain_ = v * 0.01f; break;
      case P_REVERSE: rev_ = (uint8_t)clampi(v, 0, 2); break;
      case P_DRIFT:   drift_ = v * 0.01f; break;
      case P_STEREO:  stereo_ = v * 0.01f; break;
      default: break;
    }
  }
  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *sc[] = {"CHROM", "MAJOR", "MINOR", "PENTA", "OCTAVE"};
    static const char *rv[] = {"OFF", "ON", "MISTO"};
    if (id == P_SCALE && v >= 0 && v < 5) return sc[v];
    if (id == P_REVERSE && v >= 0 && v < 3) return rv[v];
    if (id == P_VOICE2 && v >= 0 && v < 50) {
      if (v == 0) return "OFF";
      static char buf[6]; int s = v - 25, k = 0; if (s < 0) { buf[k++] = '-'; s = -s; }
      if (s >= 10) buf[k++] = (char)('0' + s / 10); buf[k++] = (char)('0' + s % 10); buf[k] = 0; return buf;
    }
    return nullptr;
  }

  void process(const float *in, float *out, uint32_t frames) {
    const float W = 720.f * pow2(grain_ * 3.f);                         // finestra: 15..120 ms
    iW_ = 1.f / W;
    const float cm = qcos(mix_), sm = qsin(mix_);
    const bool two = v2_ != 0;
    const int nAct = two ? 8 : 4;
    const float norm = (detune_ > 0.001f ? 0.72f : 1.f) * (two ? 0.72f : 1.f) * 1.15f;
    for (uint32_t n = 0; n < frames; ++n) {
      if ((tick_++ & 63u) == 0u) { updateRatios(); driftStep(); for (int k = 0; k < NS; ++k) rEff_[k] = rat_[k] * pow2(dr_[k] * (1.f / 1200.f)); }
      const float xl = in[2 * n], xr = in[2 * n + 1];
      // ingresso nel buffer + shimmer (le voci rientrano, un po' scurite)
      fbs_[0] += (fbL_ - fbs_[0]) * 0.35f; fbs_[1] += (fbR_ - fbs_[1]) * 0.35f;
      b_[0][w_] = soft(xl + fbs_[0] * shimmer_ * 0.85f);
      b_[1][w_] = soft(xr + fbs_[1] * shimmer_ * 0.85f);
      float wet[2] = {0.f, 0.f};
      for (int i = 0; i < nAct; ++i) {
        // i: bit0 = canale, bit1 = copia (0 = principale, 1 = raddoppio), bit2 = voce
        const int c = i & 1, copy = (i >> 1) & 1;
        if (copy && detune_ <= 0.001f) continue;
        const bool reverse = rev_ == 1 || (rev_ == 2 && copy);
        wet[c] += shift(i, c, W, reverse);
      }
      wet[0] *= norm; wet[1] *= norm;
      fbL_ = wet[0]; fbR_ = wet[1];
      w_ = (w_ + 1) & MASK;
      out[2 * n] = lim(xl * cm + wet[0] * sm);
      out[2 * n + 1] = lim(xr * cm + wet[1] * sm);
    }
  }
  // per le prove
  float ratio(int i) const { return rat_[i]; }

 private:
  // trasposto a due testine; alla ripartenza di una testina si cerca l'aggancio migliore
  inline float shift(int i, int c, float W, bool reverse) {
    const float r = rEff_[i];                                          // intonazione aggiornata ogni 64 campioni
    float &ph = ph_[i];
    const float inc = reverse ? (1.f + r) * iW_ : (1.f - r) * iW_;
    float prev = ph; ph += inc;
    if (ph >= 1.f) ph -= 1.f; else if (ph < 0.f) ph += 1.f;
    float y = 0.f;
    for (int h = 0; h < 2; ++h) {
      float q = ph + 0.5f * (float)h; if (q >= 1.f) q -= 1.f;
      float qp = prev + 0.5f * (float)h; if (qp >= 1.f) qp -= 1.f;
      // la testina h riparte (salto di fase) quando il suo peso e' nullo: scegli l'offset
      const bool wrapped = (inc > 0.f && q < qp - 0.5f) || (inc < 0.f && q > qp + 0.5f);
#ifndef NOSPLICE
      if (wrapped) off_[i][h] = reverse ? 0.f : splice(c, i, h, W, q);
#else
      (void)wrapped;
#endif
      const float d = 8.f + (reverse ? (1.f - q) : q) * W + off_[i][h];
      const float tq = q * 256.f; const int ti = (int)tq; const float tf = tq - (float)ti;
      y += rd(c, d) * (kWin[ti] + (kWin[ti + 1] - kWin[ti]) * tf);    // sin^2: con l'altra testina la somma e' 1
    }
    return y;
  }
  // aggancio: tra NCAND posizioni nei SEARCH campioni, quella piu' correlata con l'altra testina
  float splice(int c, int i, int h, float W, float q) {
    float qo = q + 0.5f; if (qo >= 1.f) qo -= 1.f;
    const float dOther = 8.f + qo * W + off_[i][h ^ 1];
    const float *b = b_[c];
    float best = -1e30f, bo = 0.f;
    for (int k = 0; k < NCAND; ++k) {
      const float o = (float)k * ((float)SEARCH / (float)NCAND);
      const float dNew = 8.f + q * W + o;
      int32_t pa = (int32_t)w_ - (int32_t)dOther, pb = (int32_t)w_ - (int32_t)dNew;
      float xy = 0.f, yy = 1e-9f;
      for (int t = 0; t < CL; t += 2) { const float a = b[(uint32_t)(pa - t) & MASK], bb = b[(uint32_t)(pb - t) & MASK]; xy += a * bb; yy += bb * bb; }   // un campione su due
      const float score = xy / sqrtf_(yy);
      if (score > best) { best = score; bo = o; }
    }
    return bo;
  }
  void updateRatios() {
    const int s1 = quant(interv_ - 24);
    const int s2 = v2_ ? quant(v2_ - 25) : 0;
    const float det = detune_ * 30.f;                                  // cents del raddoppio
    for (int i = 0; i < NS; ++i) {
      const int c = i & 1, copy = (i >> 1) & 1, voice = (i >> 2) & 1;
      const int semi = voice ? s2 : s1;
      // scordamenti opposti tra i canali (STEREO): principale -det/2 a sinistra e +det/2 a destra, raddoppio il contrario
      float cents = (copy ? 0.5f : -0.5f) * det; if (c) cents = cents * (1.f - 2.f * stereo_);
      rat_[i] = pow2(((float)semi + cents * 0.01f) * (1.f / 12.f));
    }
  }
  void driftStep() {                                                   // deriva lenta e casuale (fino a ~35 cents)
    for (int i = 0; i < NS; ++i) {
      if (--drCnt_[i] <= 0) { drCnt_[i] = 300 + (int32_t)(rand01() * 900.f); drT_[i] = (rand01() * 2.f - 1.f) * 35.f * drift_; }
      dr_[i] += (drT_[i] - dr_[i]) * 0.004f;
    }
  }
  int quant(int s) const {                                             // intervallo sulla scala (sotto o uguale)
    static const int8_t SC[5][12] = {
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {0, 0, 2, 2, 4, 5, 5, 7, 7, 9, 9, 11}, {0, 0, 2, 3, 3, 5, 5, 7, 8, 8, 10, 10},
      {0, 0, 2, 2, 4, 4, 4, 7, 7, 9, 9, 9}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    const int oc = s >= 0 ? s / 12 : -((-s + 11) / 12); const int pc = s - oc * 12;
    return oc * 12 + SC[scale_][pc];
  }
  inline float rd(int c, float d) const {
    const float rp = (float)w_ - d; int32_t i = (int32_t)rp; if (rp < 0.f && (float)i != rp) --i; const float f = rp - (float)i;
    const float *b = b_[c]; const float a = b[(uint32_t)i & MASK], e = b[(uint32_t)(i + 1) & MASK]; return a + (e - a) * f;
  }
  static inline float qsin(float u) { const float x = 1.5707963f * u, x2 = x * x; return x * (1.f - x2 * (0.16666667f - x2 * (0.0083333f - x2 * 0.000198f))); }
  static inline float qcos(float u) { return qsin(1.f - u); }
  static inline int clampi(int32_t v, int a, int b) { return v < a ? a : (v > b ? b : (int)v); }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }
  static inline float lim(float y) { const float a = y < 0.f ? -y : y; if (a <= 0.8f) return y; const float e = (a - 0.8f) * 5.f; const float l = 0.8f + 0.2f * e / (1.f + e); return y < 0.f ? -l : l; }
  static inline float sqrtf_(float x) { if (x <= 0.f) return 0.f; float y = x > 1.f ? x : 1.f; for (int k = 0; k < 7; ++k) y = 0.5f * (y + x / y); return y; }
  static inline float pow2(float x) { const bool ng = x < 0.f; if (ng) x = -x; const int32_t ip = (int32_t)x; const float f = x - ip;
    const float r = (float)(1u << (ip > 30 ? 30 : ip)) * (1.f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * 0.00961813f)))); return ng ? 1.f / r : r; }
  inline float rand01() { rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5; return (rng_ >> 8) * (1.f / 16777216.f); }

  float *b_[2] = {nullptr, nullptr}; uint32_t w_ = 0, tick_ = 0, rng_ = 1;
  int interv_, v2_; uint8_t scale_, rev_; float mix_, detune_, shimmer_, grain_, drift_, stereo_;
  float iW_ = 1.f / 2036.f, rEff_[NS] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
  float fbL_, fbR_, fbs_[2], ph_[NS], off_[NS][2], rat_[NS], dr_[NS], drT_[NS]; int32_t drCnt_[NS];
};
