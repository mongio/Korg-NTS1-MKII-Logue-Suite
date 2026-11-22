#pragma once
/*
 *  File: lola.h  (LOLA v0.2: coefficienti e altezza ogni 4 campioni, inviluppo per moltiplicazione)
 *  Oscillatore che abbaia, per sintesi: sorgente vocale ruvida (saw PolyBLEP + jitter + sottoarmoniche
 *  + rumore) -> 3 formanti che si aprono e chiudono -> profili di altezza e volume per ogni verso.
 *  Dedicato a Lola, bassotta a pelo duro di 7 mesi. Nessuna dipendenza da libm.
 */
#include <stdint.h>
#include "unit_osc.h"

class Lola {
 public:
  enum { P_SIZE = 0, P_ROUGH, P_CALL, P_REPEAT, P_PITCH, P_LENGTH, P_VARY, P_TONE, P_PANT, P_CRACK, P_NUM };
  enum { C_BARK = 0, C_YIP, C_ARF, C_GROWL, C_WHINE, C_HOWL, C_MIX, C_NUM };
  enum { REP_FREE_MAX = 39, REP_SYNC = 40, REP_MAX = 50 };
  static constexpr float kSR = 48000.f;

  void init() {
    size_ = 0.25f; rough_ = 0.4f; call_ = C_BARK; repeat_ = 0; natural_ = true; length_ = 0.5f;
    vary_ = 0.4f; tone_ = 0.6f; pant_ = 0.f; crack_ = 0.3f; bpm_ = 120.f;
    rng_ = 0x4C4F4C41u; held_ = 0;
    active_ = false; t_ = 0.f; rep_cnt_ = 0.f; pending_arf_ = false;
    ph_ = 0.f; odd_ = false; jit_ = 1.f; amph_ = 0.f; vph_ = 0.f; pph_ = 0.f;
    for (int i = 0; i < 3; ++i) ic1_[i] = ic2_[i] = 0.f;
    pb1_ = pb2_ = 0.f; lp1_ = lp2_ = 0.f; dcx_ = dcy_ = 0.f; pos_ = 1.f;
  }

  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_SIZE:   size_ = v * (1.f / 1023.f); break;
      case P_ROUGH:  rough_ = v * (1.f / 1023.f); break;
      case P_CALL:   call_ = (uint8_t)v; break;
      case P_REPEAT: repeat_ = (uint8_t)v; break;
      case P_PITCH:  natural_ = (v != 0); break;
      case P_LENGTH: length_ = v * 0.01f; break;
      case P_VARY:   vary_ = v * 0.01f; break;
      case P_TONE:   tone_ = v * 0.01f; break;
      case P_PANT:   pant_ = v * 0.01f; break;
      case P_CRACK:  crack_ = v * 0.01f; break;
      default: break;
    }
  }

  const char *paramStr(uint8_t id, int32_t v) {
    static const char *calls[] = {"BARK", "YIP", "ARF", "GROWL", "WHINE", "HOWL", "MIX"};
    static const char *pitch[] = {"KEY", "NATURAL"};
    static const char *divs[] = {"1-1", "1-2", "1-4.", "1-4", "1-4T", "1-8.", "1-8", "1-8T", "1-16", "1-16T", "1-32"};
    if (id == P_CALL && v >= 0 && v < C_NUM) return calls[v];
    if (id == P_PITCH && v >= 0 && v < 2) return pitch[v];
    if (id == P_REPEAT) {
      if (v <= 0) return "OFF";
      if (v >= REP_SYNC && v <= REP_MAX) return divs[v - REP_SYNC];
      return fmtHz(repHz((uint8_t)v));
    }
    return nullptr;
  }

  void setTempo(float bpm) { if (bpm > 10.f && bpm < 1000.f) bpm_ = bpm; }
  void setLfo(float l) { lfo_ = l; }   // Shape LFO -> taglia

  void noteOn(uint8_t note) {
    (void)note;
    if (note != 0xFF && held_ < 255) ++held_;
    trigger();
    rep_cnt_ = repPeriod();
  }
  void noteOff(uint8_t note) { if (note != 0xFF && held_) --held_; }
  void allNotesOff() { held_ = 0; }

  void process(float base_note, float *out, uint32_t frames) {
    // vocali (U O A E I), come BOCCA
    static const float F1[5] = {325.f, 450.f, 700.f, 530.f, 300.f};
    static const float F2[5] = {700.f, 800.f, 1220.f, 1840.f, 2300.f};
    static const float F3[5] = {2530.f, 2830.f, 2600.f, 2480.f, 3000.f};
    static const float GV[5] = {2.3f, 3.9f, 4.5f, 3.9f, 1.9f};

    // altezza di base: KEY = la nota; NATURAL = voce naturale per la taglia, trasposta (C4 = naturale)
    float nn = base_note; if (nn < 0.f) nn = 0.f; if (nn > 151.f) nn = 151.f;
    const uint8_t ni = (uint8_t)nn;
    const float f_key = osc_w0f_for_note(ni, (uint8_t)((nn - ni) * 255.f)) * kSR;
    const float sz = clampf(size_ + lfo_ * 0.25f, 0.f, 1.f);
    const float f_nat = 900.f * pow2s(-sz * 2.5f) * pow2s((base_note - 60.f) * (1.f / 12.f));
    const float f_base = natural_ ? f_nat : f_key;
    const float fscale = 1.55f * pow2s(-sz);                         // bocca piccola = formanti alte
    float x = 6.2831853f * (1500.f * pow2s(tone_ * 3.2f)) / kSR; if (x > 3.f) x = 3.f;
    const float tc = x / (1.f + x);

    for (uint32_t n = 0; n < frames; ++n) {
      // ---- ripetizione mentre il tasto e' tenuto ----
      if (held_ > 0 && repeat_ > 0) {
        rep_cnt_ -= 1.f;
        if (rep_cnt_ <= 0.f) {
          rep_cnt_ = repPeriod() * (1.f + randf() * vary_ * 0.25f);
          if (!active_ || t_ > cur_.dur * 0.7f) trigger();
        }
      }

      float s = 0.f;
      if (active_) {
        const float u = t_ / cur_.dur;                                   // 0..1 nel verso
        // profilo di altezza (semitoni) e di bocca (0 = U .. 4 = I)
        const float semi = shape3(cur_.p0, cur_.p1, cur_.p2, cur_.tp, u) + var_semi_;
        const float vow = clampf(shape3(cur_.v0, cur_.v1, cur_.v2, cur_.tv, u) + var_vow_, 0.f, 4.f);
        // inviluppo
        float env = t_ < cur_.att ? t_ / cur_.att : 1.f;
        env *= dk_env_; dk_env_ *= dk_step_;          // e^(-t*dk) per moltiplicazioni successive
        const float rel_start = cur_.dur * (1.f - cur_.rel);
        if (t_ > rel_start) env *= 1.f - (t_ - rel_start) / (cur_.dur * cur_.rel);
        env *= var_amp_;
        // stecca da cucciolo: per qualche decina di ms la voce salta all'ottava
        float crk = 1.f;
        if (crack_at_ >= 0.f && t_ >= crack_at_ && t_ < crack_at_ + crack_len_) crk = 2.f;
        // vibrato (lamento, ululato)
        vph_ += 5.3f / kSR; if (vph_ >= 1.f) vph_ -= 1.f;
        const float vib = 1.f + cur_.vib * 0.025f * sinp(vph_);
        if ((upd_ & 3u) == 0u) semi_mul_ = pow2s(semi * (1.f / 12.f));
        float f = f_base * semi_mul_ * crk * vib * jit_;
        float w = f / kSR; if (w > 0.4f) w = 0.4f;

        // ---- sorgente: saw PolyBLEP, jitter per periodo, sottoarmoniche (ruvidita') ----
        ph_ += w;
        if (ph_ >= 1.f) {
          ph_ -= 1.f; odd_ = !odd_;
          jit_ = 1.f + randf() * rough_ * cur_.rough * 0.045f;
        }
        float src = 2.f * ph_ - 1.f - blep(ph_, w);
        if (odd_) src *= 1.f - rough_ * cur_.rough * 0.45f;              // raddoppio di periodo
        const float noise = randf() * (cur_.noise * (0.5f + rough_));
        src = src * (1.f - cur_.noise * 0.5f) + noise;
        // ringhio: modulazione d'ampiezza rapida
        if (cur_.am > 0.f) { amph_ += (24.f + rough_ * 14.f) / kSR; if (amph_ >= 1.f) amph_ -= 1.f;
          src *= 1.f - cur_.am * (0.5f + 0.5f * sinp(amph_)); }

        // ---- formanti ----
        pos_ += (vow - pos_) * 0.02f;
        const int32_t i0 = (int32_t)pos_ >= 4 ? 3 : (int32_t)pos_;
        const float fr = pos_ - (float)i0;
        const float f1 = lerp(F1[i0], F1[i0 + 1], fr) * fscale, f2 = lerp(F2[i0], F2[i0 + 1], fr) * fscale;
        const float f3 = lerp(F3[i0], F3[i0 + 1], fr) * fscale;
        const float gv = lerp(GV[i0], GV[i0 + 1], fr);
        if ((upd_ & 3u) == 0u) { setBand(0, f1, 4.f); setBand(1, f2, 6.f); setBand(2, f3, 8.f); }
        ++upd_;
        const float y = band(0, src) + band(1, src) * 0.6f + band(2, src) * 0.3f;
        s = (y * gv * 0.45f + src * call_body_) * env * call_gain_;

        t_ += 1.f / kSR;
        if (t_ >= cur_.dur) {
          active_ = false;
          if (pending_arf_) { pending_arf_ = false; gap_ = 0.045f; }     // secondo "arf" dopo una pausa
        }
      } else if (gap_ > 0.f) {
        gap_ -= 1.f / kSR;
        if (gap_ <= 0.f) { gap_ = 0.f; startCall(C_BARK, 0.62f, -2.5f); }
      }

      // ---- fiatone tra un verso e l'altro (tasto tenuto) ----
      if (pant_ > 0.f && held_ > 0 && !active_) {
        pph_ += 3.6f / kSR; if (pph_ >= 1.f) pph_ -= 1.f;
        const float e = sinp(pph_ * 2.f >= 1.f ? pph_ * 2.f - 1.f : pph_ * 2.f);   // inspiro / espiro
        const float bn = randf();
        pb1_ += (bn - pb1_) * 0.35f; pb2_ += (pb1_ - pb2_) * 0.35f;
        s += (pb1_ - pb2_) * (e > 0.f ? e : 0.f) * pant_ * 0.9f;
      }

      // ---- tono, blocco DC, limitatore morbido ----
      lp1_ += (s - lp1_) * tc; lp2_ += (lp1_ - lp2_) * tc; s = lp2_;
      const float dy = s - dcx_ + 0.995f * dcy_; dcx_ = s; dcy_ = dy; s = dy;
      const float a = s < 0.f ? -s : s;
      if (a > 0.8f) { const float e2 = (a - 0.8f) * 5.f; const float l = 0.8f + 0.2f * e2 / (1.f + e2); s = s < 0.f ? -l : l; }
      out[n] = s;
    }
  }

 private:
  struct Call { float dur, p0, p1, p2, tp, v0, v1, v2, tv, att, dk, rel, noise, rough, am, vib; };
  float call_gain_ = 1.f, call_body_ = 0.12f;

  void trigger() {
    uint8_t c = call_;
    if (c == C_MIX) { const uint32_t r = nextRand() % 4; c = r < 2 ? C_BARK : (r == 2 ? C_YIP : C_ARF); }
    if (c == C_ARF) { startCall(C_BARK, 0.55f, 1.f); pending_arf_ = true; }
    else startCall(c, 1.f, 0.f);
  }

  void startCall(uint8_t c, float dur_mul, float semi_off) {
    //                    dur    p0     p1    p2    tp    v0   v1   v2   tv   att    dk    rel  noise rough  am   vib
    static const Call T[6] = {
        {0.16f, -3.f, 2.f, -7.f, 0.22f, 0.6f, 3.0f, 1.6f, 0.30f, 0.004f, 6.f, 0.35f, 0.35f, 1.0f, 0.f, 0.f},   // BARK
        {0.075f, 6.f, 9.f, 3.f, 0.30f, 2.5f, 3.6f, 3.0f, 0.30f, 0.003f, 9.f, 0.40f, 0.15f, 0.4f, 0.f, 0.f},   // YIP
        {0.16f, -3.f, 2.f, -7.f, 0.22f, 0.6f, 3.0f, 1.6f, 0.30f, 0.004f, 6.f, 0.35f, 0.35f, 1.0f, 0.f, 0.f},   // ARF (= due BARK)
        {0.90f, -12.f, -11.f, -13.f, 0.50f, 1.0f, 1.4f, 0.8f, 0.50f, 0.060f, 0.6f, 0.25f, 0.60f, 2.0f, 0.55f, 0.f}, // GROWL
        {0.80f, 11.f, 14.f, 8.f, 0.40f, 3.6f, 3.9f, 3.2f, 0.50f, 0.040f, 0.8f, 0.30f, 0.08f, 0.1f, 0.f, 0.3f}, // WHINE
        {1.80f, 2.f, 12.f, 9.f, 0.30f, 2.6f, 1.2f, 0.4f, 0.50f, 0.150f, 0.2f, 0.30f, 0.05f, 0.1f, 0.f, 0.5f},  // HOWL
    };
    cur_ = T[c < 6 ? c : 0];
    static const float CG[6] = {1.f, 1.35f, 1.f, 1.5f, 1.8f, 0.95f};    // compensazione di livello per verso
    static const float CB[6] = {0.12f, 0.15f, 0.12f, 0.12f, 0.45f, 0.15f};   // suono diretto (il lamento ne ha di piu')
    call_gain_ = CG[c < 6 ? c : 0]; call_body_ = CB[c < 6 ? c : 0];
    const float lm = 0.5f + length_;                                      // LENGTH: 0.5x .. 1.5x
    cur_.dur *= lm * dur_mul * (1.f + randf() * vary_ * 0.35f);
    if (cur_.dur < 0.03f) cur_.dur = 0.03f;
    var_semi_ = semi_off + randf() * vary_ * 2.5f;
    var_vow_ = randf() * vary_ * 0.5f;
    var_amp_ = 1.f - rand01() * vary_ * 0.3f;
    crack_at_ = -1.f;
    if (rand01() < crack_ * 0.8f) { crack_at_ = cur_.dur * (0.2f + 0.5f * rand01()); crack_len_ = 0.02f + 0.025f * rand01(); }
    t_ = 0.f; active_ = true; gap_ = 0.f; jit_ = 1.f;
    dk_env_ = 1.f; dk_step_ = expo(-cur_.dk / kSR); upd_ = 0;
    pos_ = cur_.v0;
  }

  float repPeriod() const {
    if (repeat_ >= REP_SYNC) {
      static const float q[] = {4.f, 2.f, 1.5f, 1.f, 0.6667f, 0.75f, 0.5f, 0.3333f, 0.25f, 0.1667f, 0.125f};
      return q[repeat_ - REP_SYNC] * (60.f / bpm_) * kSR;
    }
    return kSR / repHz(repeat_ ? repeat_ : 1);
  }
  static float repHz(uint8_t i) { return 0.5f * pow2s((float)(i - 1) * (4.f / 38.f)); }   // 0,5 .. 8 Hz

  // ---------- utilita' ----------
  inline void setBand(int k, float fc, float q) {
    if (fc > 16000.f) fc = 16000.f;
    const float x = 3.14159265f * fc / kSR, x2 = x * x;
    const float g = x * (1.f + x2 * (0.3333333f + x2 * (0.1333333f + x2 * 0.0539683f)));
    const float kk = 1.f / q, a1 = 1.f / (1.f + g * (g + kk));
    bk_[k] = kk; ba1_[k] = a1; ba2_[k] = g * a1; ba3_[k] = g * g * a1;
  }
  inline float band(int k, float v0) {
    const float kk = bk_[k], a1 = ba1_[k], a2 = ba2_[k], a3 = ba3_[k];
    float &i1 = ic1_[k], &i2 = ic2_[k];
    const float v3 = v0 - i2, v1 = a1 * i1 + a2 * v3, v2 = i2 + a2 * i1 + a3 * v3;
    i1 = 2.f * v1 - i1; i2 = 2.f * v2 - i2;
    return v1 * kk;
  }
  static inline float shape3(float a, float b, float c, float tp, float u) {
    return u < tp ? a + (b - a) * (u / tp) : b + (c - b) * ((u - tp) / (1.f - tp));
  }
  static inline float blep(float t, float dt) {
    if (t < dt) { const float x = t / dt; return x + x - x * x - 1.f; }
    if (t > 1.f - dt) { const float x = (t - 1.f) / dt; return x * x + x + x + 1.f; }
    return 0.f;
  }
  static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
  static inline float clampf(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }
  static inline float sinp(float ph) { float x = ph < 0.5f ? ph * 4.f - 1.f : 3.f - ph * 4.f; return x * (1.5f - 0.5f * x * x); }
  static inline float pow2p(float x) { const int32_t ip = (int32_t)x; const float f = x - ip;
    return (float)(1u << ip) * (1.f + f * (0.6931472f + f * (0.2402265f + f * 0.0555041f))); }
  static inline float pow2s(float x) { return x >= 0.f ? pow2p(x) : 1.f / pow2p(-x); }   // 2^x, |x| < 30
  static inline float expo(float x) { return pow2s(x * 1.4426950f); }                       // e^x
  inline uint32_t nextRand() { uint32_t r = rng_; r ^= r << 13; r ^= r >> 17; r ^= r << 5; rng_ = r; return r; }
  inline float rand01() { return (nextRand() >> 8) * (1.f / 16777216.f); }
  inline float randf() { return rand01() * 2.f - 1.f; }
  const char *fmtHz(float hz) {
    char *p = buf_; const int32_t d = (int32_t)(hz * 10.f + 0.5f);
    if (d >= 100) { *p++ = (char)('0' + d / 100); }
    *p++ = (char)('0' + (d / 10) % 10); *p++ = '.'; *p++ = (char)('0' + d % 10); *p = 0;
    return buf_;
  }

  float size_, rough_, length_, vary_, tone_, pant_, crack_, bpm_, lfo_ = 0.f;
  uint8_t call_, repeat_, held_; bool natural_;
  uint32_t rng_;
  Call cur_; bool active_, pending_arf_, odd_;
  float t_, gap_ = 0.f, rep_cnt_, var_semi_ = 0.f, var_vow_ = 0.f, var_amp_ = 1.f, crack_at_ = -1.f, crack_len_ = 0.f;
  float ph_, jit_, amph_, vph_, pph_, pos_;
  float ic1_[3], ic2_[3], pb1_, pb2_, lp1_, lp2_, dcx_, dcy_;
  char buf_[8];
  float dk_env_ = 1.f, dk_step_ = 1.f, semi_mul_ = 1.f, bk_[3] = {1.f, 1.f, 1.f}, ba1_[3] = {0, 0, 0}, ba2_[3] = {0, 0, 0}, ba3_[3] = {0, 0, 0};
  uint32_t upd_ = 0;
};
