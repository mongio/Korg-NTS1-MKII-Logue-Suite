#pragma once
/*
 *  MAREA v0.2 - una modulazione che va a onde e fa ruotare il tipo di effetto (v0.2: effetti molto piu' decisi,
 *  intensita' da zero al massimo con l'onda, una stazione nuova a ogni cresta, periodo 0,5-16 s).
 *  Il mare (onda lunga + onda secondaria + moto ondoso + onde anomale) decide QUANTO agisce l'effetto del momento
 *  (altezza dell'onda) e QUALE effetto agisce (le onde fanno ruotare le stazioni, con dissolvenza):
 *  FLANGE, CHORUS, RISACCA (filtro), SALE (saturazione), RING (ring mod), ROVINA (degrado progressivo), ECO.
 *  Si calcolano solo le due stazioni attive. Nessuna dipendenza da libm.
 */
#include <stdint.h>
class Marea {
 public:
  enum { P_MARE = 0, P_PERIODO, P_ROTA, P_STAZIONI, P_STAZ, P_AMOUNT, P_STEREO, P_MIX, P_NUM };
  enum { S_FLANGE = 0, S_CHORUS, S_RISACCA, S_SALE, S_RING, S_ROVINA, S_ECO, S_NUM };
  enum { R_ONDA = 0, R_MAREA, R_CASO, R_FISSO };
  enum { FN = 512, FM = FN - 1, CN = 2048, CM = CN - 1, EN = 16384, EM = EN - 1 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kFloats = 2 * (FN + CN + EN);

  void init(float *mem) {
    fL_ = mem; fR_ = fL_ + FN; cL_ = fR_ + FN; cR_ = cL_ + CN; eL_ = cR_ + CN; eR_ = eL_ + EN;
    mare_ = 0.5f; per_ = 0.5f; rota_ = R_ONDA; set_ = 0; staz_ = S_FLANGE; amount_ = 0.8f; stereo_ = 0.7f; mix_ = 1.f;
    clear();
  }
  void clear() {
    for (uint32_t i = 0; i < kFloats; ++i) fL_[i] = 0.f;
    w_ = 0; tick_ = 0; rng_ = 0x3A2E0F1Bu;
    ph1_ = 0.f; ph2_ = 0.37f; chop_ = 0.f; chopT_ = 0.f; chopCnt_ = 0; rogue_ = 0.f; H_ = Hs_ = 0.3f; prevPh_ = 0.f;
    cur_ = 0; nxt_ = 0; xf_ = 1.f; xfStep_ = 0.f; idx_ = 0; tideAcc_ = 0.f; rovina_ = 0.f;
    lfoA_ = 0.f; lfoB_ = 0.25f; ringPh_ = 0.f; flfb_[0] = flfb_[1] = 0.f;
    for (int c = 0; c < 2; ++c) { lp_[c] = bp_[c] = 0.f; sh_[c] = 0.f; shCnt_[c] = 0; rlp_[c] = 0.f; slp_[c] = 0.f; }
    dF_ = 96.f; dC_ = 700.f; dE_ = 7000.f;
    applyStart();
  }
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_MARE:     mare_ = v * (1.f / 1023.f); break;
      case P_PERIODO:  per_ = v * (1.f / 1023.f); break;
      case P_ROTA:     rota_ = (uint8_t)clampi(v, 0, 3); if (rota_ == R_FISSO) applyStart(); break;
      case P_STAZIONI: set_ = (uint8_t)clampi(v, 0, 2); applyStart(); break;
      case P_STAZ:     staz_ = (uint8_t)clampi(v, 0, 6); applyStart(); break;
      case P_AMOUNT:   amount_ = v * 0.01f; break;
      case P_STEREO:   stereo_ = v * 0.01f; break;
      case P_MIX:      mix_ = v * 0.01f; break;
      default: break;
    }
  }
  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *rot[] = {"ONDA", "MAREA", "CASO", "FISSO"};
    static const char *sets[] = {"TUTTE", "PULITE", "SPORCHE"};
    static const char *st[] = {"FLANGE", "CHORUS", "RISACCA", "SALE", "RING", "ROVINA", "ECO"};
    if (id == P_ROTA && v >= 0 && v < 4) return rot[v];
    if (id == P_STAZIONI && v >= 0 && v < 3) return sets[v];
    if (id == P_STAZ && v >= 0 && v < 7) return st[v];
    return nullptr;
  }
  int current() const { return list()[cur_ % count()]; }

  void process(const float *in, float *out, uint32_t frames) {
    for (uint32_t n = 0; n < frames; ++n) {
      if ((tick_++ & 15u) == 0u) sea();
      Hs_ += (H_ - Hs_) * 0.004f; HsL_ += (HL_ - HsL_) * 0.004f; HsR_ += (HR_ - HsR_) * 0.004f;   // altezze, morbide ma reattive
      const float xl = in[2 * n], xr = in[2 * n + 1];
      // tutte le linee ricevono sempre l'ingresso (cosi' una stazione che arriva ha gia' la sua "memoria")
      fL_[w_ & FM] = soft(xl + flfb_[0]); fR_[w_ & FM] = soft(xr + flfb_[1]);
      cL_[w_ & CM] = xl; cR_[w_ & CM] = xr;
      // ECO: scritto con il suo feedback nella stazione stessa (vedi st()); qui solo l'ingresso se l'eco non e' attivo
      const bool ecoOn = (stA_ == S_ECO && xf_ < 0.999f) || stB_ == S_ECO;
      if (!ecoOn) { eL_[w_ & EM] = xl; eR_[w_ & EM] = xr; }
      flfb_[0] = flfb_[1] = 0.f;
      // dissolvenza tra le due stazioni attive (potenza costante)
      if (xfStep_ != 0.f) { xf_ += xfStep_; if (xf_ >= 1.f) { xf_ = 1.f; xfStep_ = 0.f; cur_ = nxt_; } }
      stA_ = list()[cur_ % count()]; stB_ = list()[nxt_ % count()];
      float wl, wr;
      if (stA_ == stB_ || xfStep_ == 0.f) {                            // una sola stazione, a piena intensita'
        float aL = xl, aR = xr; st(stA_ == stB_ ? stA_ : stB_, aL, aR, xl, xr); wl = aL; wr = aR;
      } else {                                                          // passaggio: quella che svanisce e quella che arriva
        float aL = xl, aR = xr, bL = xl, bR = xr;
        const float gA = cosq(xf_), gB = sinq(xf_);
        st(stA_, aL, aR, xl, xr); st(stB_, bL, bR, xl, xr);
        wl = aL * gA + bL * gB; wr = aR * gA + bR * gB;
      }
      if (ecoOn && ecoWritten_ == false) { eL_[w_ & EM] = xl; eR_[w_ & EM] = xr; }
      ecoWritten_ = false;
      ++w_;
      // l'onda che attraversa lo stereo sposta anche il suono: piu' forte dal lato dove arriva la cresta
      const float pv = 0.9f * stereo_ * (HsL_ - HsR_);
      const float gl = 1.f + pv, gr = 1.f - pv;
      out[2 * n] = lim(xl * (1.f - mix_) + wl * gl * mix_);
      out[2 * n + 1] = lim(xr * (1.f - mix_) + wr * gr * mix_);
    }
  }

 private:
  // ---------- le stazioni ----------
  void st(int s, float &l, float &r, float xl, float xr) {
    // ogni canale ha la sua altezza d'onda: l'onda attraversa lo stereo (destra in ritardo di un quarto d'onda x STEREO)
    const float x2[2] = {xl, xr}; float y2[2];
    const float Hc[2] = {HsL_, HsR_};
    for (int c = 0; c < 2; ++c) {
      const float H = Hc[c], I = amount_ * H, x = x2[c];
      float y = x;
      switch (s) {
        case S_FLANGE: {                                               // 0,25..7,75 ms con l'onda, feedback negativo ("jet")
          const float dT = (0.25f + 7.5f * H) * 48.f; dFc_[c] += (dT - dFc_[c]) * 0.004f;
          const float d = rd(c ? fR_ : fL_, FM, dFc_[c]);
          flfb_[c] = -d * (0.3f + 0.6f * amount_);
          y = (x + d * (0.35f + 0.65f * I)) * 0.75f;
        } break;
        case S_CHORUS: {
          const float dep = (2.f + 11.f * I) * 48.f;
          const float m = sinp(wrap(lfoA_ + 0.5f * stereo_ * c)) * 0.7f + sinp(wrap(lfoB_ + 0.33f * stereo_ * c)) * 0.3f;
          const float d = rd(c ? cR_ : cL_, CM, 900.f + dep * m);
          y = x * (0.75f - 0.35f * I) + d * (0.45f + 0.5f * I);
        } break;
        case S_RISACCA: {                                              // 150 Hz (cavo) .. ~14 kHz (cresta), risonante
          const float e = 6.6f * (H * amount_ + (1.f - amount_));
          float g = 3.14159265f * (150.f * pow2(e < 0.f ? 0.f : e)) / kSR; if (g > 0.95f) g = 0.95f; g = g / (1.f + g);
          y = svf(c, x, g, 1.3f - 1.22f * amount_);
        } break;
        case S_SALE: {                                                 // saturazione vera, livello contenuto
          const float d = 1.f + 16.f * I, b = 0.3f * I, mk = (1.f + 1.2f * I) / (1.f + 13.f * I);
          y = (soft(x * d + b) - soft(b)) * mk; slp_[c] += (y - slp_[c]) * 0.45f; y = slp_[c];
        } break;
        case S_RING: {                                                 // portante 20 Hz .. ~2,5 kHz con l'onda
          const float f = 20.f * pow2(7.f * H);
          ringPhc_[c] += f / kSR; if (ringPhc_[c] >= 1.f) ringPhc_[c] -= 1.f;
          const float dpt = 0.4f + 0.6f * I; y = x * (1.f - dpt + dpt * sinp(ringPhc_[c]));
        } break;
        case S_ROVINA: {                                               // degrado progressivo (rovina_)
          const float R = rovina_ * amount_ * (0.6f + 0.4f * H);
          const int hold = 1 + (int)(R * 31.f);
          const float q = pow2(15.f - 12.5f * R);
          if (++shCnt_[c] >= hold) { shCnt_[c] = 0; const float v = x * q; sh_[c] = (float)(int32_t)(v + (v >= 0.f ? 0.5f : -0.5f)) / q; }
          rlp_[c] += (sh_[c] - rlp_[c]) * (0.9f - 0.8f * R); y = rlp_[c];
        } break;
        default: {                                                     // ECO: 60..300 ms, rimbalzo tra i canali con STEREO
          const float dT = (60.f + 240.f * per_) * 48.f * (c ? 1.f + 0.25f * stereo_ : 1.f); dEc_[c] += (dT - dEc_[c]) * 0.0005f;
          const float d = rd(c ? eR_ : eL_, EM, dEc_[c]), o = rd(c ? eL_ : eR_, EM, dEc_[c]);
          const float fb = 0.35f + 0.4f * I;
          (c ? eR_ : eL_)[w_ & EM] = soft(x + (d * (1.f - stereo_ * 0.5f) + o * stereo_ * 0.5f) * fb);
          ecoWritten_ = true;
          y = x * 0.8f + d * (0.4f + 0.8f * I);
        } break;
      }
      y2[c] = y;
    }
    l = y2[0]; r = y2[1];
  }
  // ---------- il mare (ogni 16 campioni) ----------
  void sea() {
    const float dt = 16.f / kSR;
    const float P = 0.5f * pow2(5.f * per_);                         // periodo dell'onda lunga: 0,5..16 s
    ph1_ += dt / P; ph2_ += dt / (P * 0.61f);
    bool crest = false;
    if (ph1_ >= 1.f) { ph1_ -= 1.f; }
    if (ph2_ >= 1.f) ph2_ -= 1.f;
    // cresta dell'onda lunga (fase 0,25 = massimo del seno)
    if (prevPh_ < 0.25f && ph1_ >= 0.25f) crest = true;
    prevPh_ = ph1_;
    // moto ondoso: rumore lisciato piu' veloce, piu' forte con il mare grosso
    if (--chopCnt_ <= 0) { chopCnt_ = (int32_t)(P * 0.18f / dt) + 1; chopT_ = rand01() * 2.f - 1.f; }
    chop_ += (chopT_ - chop_) * 0.01f;
    // onde anomale: ogni tanto, piu' spesso con la burrasca
    if (crest && rand01() < 0.04f + 0.25f * mare_ * mare_) rogue_ = 1.f;
    rogue_ *= 1.f - dt / (P * 0.6f);
    const float amp = 0.3f + 0.25f * mare_;                            // anche in bonaccia l'onda e' piena
    const float common = amp * (0.2f + 0.6f * mare_) * chop_ + 0.35f * rogue_ * (0.5f + 0.5f * mare_);
    float h = 0.45f + amp * (0.75f * sinp(ph1_) + 0.25f * sinp(ph2_)) + common;
    H_ = h < 0.f ? 0.f : (h > 1.f ? 1.f : h);
    // l'onda attraversa lo stereo: sinistra un ottavo d'onda prima, destra un ottavo dopo (x STEREO)
    const float sh = 0.125f * stereo_;
    float hl = 0.45f + amp * (0.75f * sinp(wrap1(ph1_ + sh)) + 0.25f * sinp(wrap1(ph2_ + sh))) + common;
    float hr = 0.45f + amp * (0.75f * sinp(wrap1(ph1_ - sh + 1.f)) + 0.25f * sinp(wrap1(ph2_ - sh + 1.f))) + common;
    HL_ = hl < 0.f ? 0.f : (hl > 1.f ? 1.f : hl); HR_ = hr < 0.f ? 0.f : (hr > 1.f ? 1.f : hr);
    // rotazione delle stazioni
    const int N = count();
    if (rota_ != R_FISSO && xfStep_ == 0.f && N > 1) {
      int go = 0;
      if (rota_ == R_ONDA && crest) go = rogue_ > 0.9f ? 2 : 1;
      else if (rota_ == R_CASO && crest) go = 1 + (int)(rand01() * (float)(N - 1));
      else if (rota_ == R_MAREA) { tideAcc_ += dt / (P * 3.f); if (tideAcc_ >= 1.f) { tideAcc_ -= 1.f; go = 1; } }
      if (go) { nxt_ = (cur_ + go) % N; xf_ = 0.f; xfStep_ = 1.f / (kSR * (0.12f + 0.2f * P)); }   // dissolvenza: un quinto d'onda
    }
    // ROVINA: si consuma mentre la marea ci si ferma, si ripulisce piano quando passa oltre
    const bool onRovina = (stA_ == S_ROVINA && xf_ < 0.5f) || (stB_ == S_ROVINA && xf_ >= 0.5f) || (cur_ % N == idxOf(S_ROVINA) && xfStep_ == 0.f);
    if (onRovina) rovina_ += dt * (0.12f + 0.35f * Hs_ * (0.5f + mare_)); else rovina_ -= dt * 0.06f;
    rovina_ = rovina_ < 0.f ? 0.f : (rovina_ > 1.f ? 1.f : rovina_);
    // LFO del chorus
    lfoA_ += 0.31f * dt; if (lfoA_ >= 1.f) lfoA_ -= 1.f; lfoB_ += 0.19f * dt; if (lfoB_ >= 1.f) lfoB_ -= 1.f;
  }
  const uint8_t *list() const {
    static const uint8_t L[3][7] = {{S_FLANGE, S_CHORUS, S_RISACCA, S_SALE, S_RING, S_ROVINA, S_ECO}, {S_FLANGE, S_CHORUS, S_RISACCA, S_ECO, 0, 0, 0}, {S_SALE, S_RING, S_ROVINA, 0, 0, 0, 0}};
    return L[set_];
  }
  int count() const { static const int C[3] = {7, 4, 3}; return C[set_]; }
  int idxOf(int s) const { const uint8_t *L = list(); for (int i = 0; i < count(); ++i) if (L[i] == s) return i; return -1; }
  void applyStart() {   // stazione di partenza (o fissa): STAZ, se fa parte dell'insieme; altrimenti la prima
    int i = idxOf(staz_); if (i < 0) i = 0;
    cur_ = nxt_ = i; xf_ = 1.f; xfStep_ = 0.f; stA_ = stB_ = list()[i];
  }
  inline float rd(const float *b, uint32_t m, float d) const {      // lettura con interpolazione lineare
    const float rp = (float)w_ - d; int32_t i = (int32_t)rp; if (rp < 0.f && (float)i != rp) --i; const float f = rp - (float)i;
    const float a = b[(uint32_t)i & m], c = b[(uint32_t)(i + 1) & m]; return a + (c - a) * f;
  }
  inline float svf(int c, float x, float g, float k) {
    const float hp = (x - (k + g) * bp_[c] - lp_[c]) / (1.f + g * (k + g)); const float bp = g * hp + bp_[c]; bp_[c] = g * hp + bp; const float lp = g * bp + lp_[c]; lp_[c] = g * bp + lp; return soft(lp);
  }
  // dissolvenza a potenza costante senza spigoli: smoothstep e poi seno/coseno di un quarto di giro
  static inline float qsin(float u) { const float x = 1.5707963f * u, x2 = x * x; return x * (1.f - x2 * (0.16666667f - x2 * (0.0083333f - x2 * 0.000198f))); }
  static inline float ss(float x) { return x * x * (3.f - 2.f * x); }
  static inline float cosq(float x) { return qsin(1.f - ss(x)); }
  static inline float sinq(float x) { return qsin(ss(x)); }
  static inline float sqrtf_(float x) { if (x <= 0.f) return 0.f; float y = x > 1.f ? x : 1.f; for (int k = 0; k < 6; ++k) y = 0.5f * (y + x / y); return y; }
  static inline int clampi(int32_t v, int a, int b) { return v < a ? a : (v > b ? b : (int)v); }
  static inline float wrap(float p) { return p >= 1.f ? p - 1.f : p; }
  static inline float wrap1(float p) { return p - (float)(int32_t)p; }
  static inline float sinp(float p) { const float t = p < 0.5f ? p * 4.f - 1.f : 3.f - p * 4.f; return -t * (1.5f - 0.5f * t * t); }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }
  static inline float lim(float y) { const float a = y < 0.f ? -y : y; if (a <= 0.8f) return y; const float e = (a - 0.8f) * 5.f; const float l = 0.8f + 0.2f * e / (1.f + e); return y < 0.f ? -l : l; }
  static inline float pow2(float x) { const bool ng = x < 0.f; if (ng) x = -x; const int32_t ip = (int32_t)x; const float f = x - ip;
    const float r = (float)(1u << (ip > 30 ? 30 : ip)) * (1.f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * 0.00961813f)))); return ng ? 1.f / r : r; }
  inline float rand01() { rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5; return (rng_ >> 8) * (1.f / 16777216.f); }

  float *fL_, *fR_, *cL_, *cR_, *eL_, *eR_; uint32_t w_ = 0, tick_ = 0, rng_ = 1;
  float mare_, per_, amount_, stereo_, mix_; uint8_t rota_, set_, staz_;
  float ph1_, ph2_, chop_, chopT_, rogue_, H_, Hs_, prevPh_, tideAcc_, rovina_; int32_t chopCnt_;
  int cur_, nxt_, idx_, stA_ = 0, stB_ = 0; float xf_, xfStep_; bool ecoWritten_ = false;
  float HL_ = 0.3f, HR_ = 0.3f, HsL_ = 0.3f, HsR_ = 0.3f, dFc_[2] = {96.f, 96.f}, dEc_[2] = {7000.f, 7000.f}, ringPhc_[2] = {0.f, 0.25f};
  float lfoA_, lfoB_, ringPh_, flfb_[2], lp_[2], bp_[2], sh_[2], rlp_[2], slp_[2], dF_, dC_, dE_; int shCnt_[2];
};
