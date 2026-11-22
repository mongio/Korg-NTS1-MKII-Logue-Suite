#pragma once
/*
 *  STORMO v0.1 - chorus "a stormo": il suono viene moltiplicato in 2..8 copie, ognuna e' un uccello di uno stormo
 *  simulato (coesione, allineamento, separazione, vagare, falco). Posizione orizzontale = stereo, profondita' =
 *  ritardo (e scurimento); il Doppler nasce da solo quando un uccello si avvicina o si allontana.
 *  Nessuna dipendenza da libm.
 */
#include <stdint.h>
class Stormo {
 public:
  enum { P_SPEED = 0, P_DEPTH, P_BIRDS, P_COH, P_PRED, P_WIDTH, P_SYNC, P_TONE, P_FB, P_MIX, P_NUM };
  enum { S_OFF = 0, S_BEAT, S_BAR };
  enum { MAXB = 8, N = 4096, MASK = N - 1 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kFloats = N;

  void init(float *mem) {
    buf_ = mem; speed_ = 0.35f; depth_ = 0.5f; nb_ = 6; coh_ = 0.5f; pred_ = 0.3f; width_ = 0.8f; sync_ = S_OFF; tone_ = 0.3f; fb_ = 0.f; mix_ = 0.5f;
    bpm_ = 120.f; clear();
  }
  void clear() {
    for (uint32_t i = 0; i < N; ++i) buf_[i] = 0.f;
    w_ = 0; tick_ = 0; rng_ = 0x5703B0F1u; fbs_ = 0.f; atk_ = 0.f; ppq_ = 0; pendAtk_ = false;
    for (int k = 0; k < MAXB; ++k) {   // stormo iniziale: un gruppo compatto con piccole velocita' casuali
      px_[k] = (rand01() - 0.5f) * 0.4f; py_[k] = (rand01() - 0.5f) * 0.4f;
      vx_[k] = (rand01() - 0.5f) * 0.3f; vy_[k] = (rand01() - 0.5f) * 0.3f;
      wx_[k] = wy_[k] = 0.f; d_[k] = dT_[k] = dS_[k] = 0.f; lp_[k] = 0.f; gl_[k] = gr_[k] = 0.7f; glT_[k] = grT_[k] = 0.7f; tc_[k] = 0.5f;
    }
    ax_ = ay_ = 0.f; for (int k = 0; k < MAXB; ++k) { d_[k] = target(k); dT_[k] = d_[k]; }
  }
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_SPEED: speed_ = v * (1.f / 1023.f); break;
      case P_DEPTH: depth_ = v * (1.f / 1023.f); break;
      case P_BIRDS: nb_ = 2 + (v < 0 ? 0 : (v > 6 ? 6 : v)); break;     // 0..6 -> 2..8 uccelli
      case P_COH:   coh_ = v * 0.01f; break;
      case P_PRED:  pred_ = v * 0.01f; break;
      case P_WIDTH: width_ = v * 0.01f; break;
      case P_SYNC:  sync_ = (uint8_t)(v < 0 ? 0 : (v > 2 ? 2 : v)); break;
      case P_TONE:  tone_ = v * 0.01f; break;
      case P_FB:    fb_ = v * 0.01f; break;
      case P_MIX:   mix_ = v * 0.01f; break;
      default: break;
    }
  }
  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *sy[] = {"OFF", "BEAT", "BAR"};
    static const char *bi[] = {"2", "3", "4", "5", "6", "7", "8"};
    if (id == P_SYNC && v >= 0 && v < 3) return sy[v];
    if (id == P_BIRDS && v >= 0 && v < 7) return bi[v];
    return nullptr;
  }
  void setTempo(float bpm) { bpm_ = bpm < 30.f ? 30.f : (bpm > 300.f ? 300.f : bpm); }
  void tick4ppqn() {                                     // 4 impulsi per quarto: battuta = 4, misura = 16
    ++ppq_;
    if (sync_ == S_BEAT && (ppq_ & 3u) == 0u) pendAtk_ = true;
    if (sync_ == S_BAR && (ppq_ & 15u) == 0u) pendAtk_ = true;
  }

  void process(const float *in, float *out, uint32_t frames) {
    const float m = mix_, mk = 1.f / sqrtf_((1.f - m) * (1.f - m) + m * m);
    const float wn = 1.5f / sqrtf_((float)nb_);            // +3,5 dB: il wet torna vicino al livello del bypass
    for (uint32_t n = 0; n < frames; ++n) {
      if ((tick_++ & 31u) == 0u) fly();
      const float xl = in[2 * n], xr = in[2 * n + 1];
      float sl = 0.f, sr = 0.f, mono = 0.f;
      for (int k = 0; k < nb_; ++k) {
        d_[k] += dS_[k];                                  // ritardo che scorre campione per campione (Doppler pulito)
        gl_[k] += (glT_[k] - gl_[k]) * 0.004f; gr_[k] += (grT_[k] - gr_[k]) * 0.004f;
        float v = readH(d_[k]);
        lp_[k] += (v - lp_[k]) * tc_[k]; v = lp_[k];      // uccelli lontani piu' scuri
        sl += v * gl_[k]; sr += v * gr_[k]; mono += v;
      }
      sl *= wn; sr *= wn; mono *= wn;
      // ingresso nel buffer (mono) con un po' di rigenerazione
      fbs_ = soft(mono * fb_ * 0.6f);
      buf_[w_] = soft((xl + xr) * 0.5f + fbs_);
      w_ = (w_ + 1) & MASK;
      out[2 * n] = lim((xl * (1.f - m) + sl * m) * mk);
      out[2 * n + 1] = lim((xr * (1.f - m) + sr * m) * mk);
    }
  }

 private:
  // ---------- il volo (ogni 32 campioni) ----------
  void fly() {
    float sp = 0.15f * pow2(speed_ * 4.5f);               // scala dei tempi del volo
    if (sync_ != S_OFF) sp *= bpm_ / 120.f;                // a tempo: piu' veloce con il tempo del NTS-1
    const float dt = 32.f / kSR * sp;
    // centro e velocita' media dello stormo
    // direzione d'insieme che cambia lentamente (lo stormo attraversa il cielo)
    gph_ += dt * 0.31f; if (gph_ > 6.2831853f) gph_ -= 6.2831853f; gph2_ += dt * 0.17f; if (gph2_ > 6.2831853f) gph2_ -= 6.2831853f;
    gx_ = sinp(gph_) + 0.5f * sinp(gph2_ * 1.3f + 1.f); gy_ = sinp(gph_ + 1.9f) * 0.8f + 0.4f * sinp(gph2_ + 2.f);
    float cx = 0.f, cy = 0.f, mvx = 0.f, mvy = 0.f;
    for (int k = 0; k < nb_; ++k) { cx += px_[k]; cy += py_[k]; mvx += vx_[k]; mvy += vy_[k]; }
    const float inv = 1.f / (float)nb_; cx *= inv; cy *= inv; mvx *= inv; mvy *= inv;
    // il falco: libero (probabilita') o sulle battute (SYNC)
    bool attack = false;
    if (sync_ == S_OFF) { if (rand01() < pred_ * pred_ * 0.0016f * sp) attack = true; }
    else if (pendAtk_) { pendAtk_ = false; if (rand01() < pred_) attack = true; }
    if (attack && atk_ < 0.35f) {                           // il falco piomba vicino al centro dello stormo
      atk_ = 1.f; ax_ = cx + (rand01() - 0.5f) * 0.3f; ay_ = cy + (rand01() - 0.5f) * 0.3f;
    }
    atk_ *= 1.f - 1.4f * (32.f / kSR); if (atk_ < 0.f) atk_ = 0.f;   // l'attacco dura ~0,7 s (tempo reale)
    const float kc = 0.25f + 1.6f * coh_, ka = 1.0f, ks = 0.03f, kh = 0.45f;
    const float vmax = 1.1f, vmin = 0.25f;
    for (int k = 0; k < nb_; ++k) {
      float fx = kc * (cx - px_[k]) + ka * (mvx - vx_[k]);
      float fy = kc * (cy - py_[k]) + ka * (mvy - vy_[k]);
      for (int j = 0; j < nb_; ++j) {                     // separazione
        if (j == k) continue;
        const float dx = px_[k] - px_[j], dy = py_[k] - py_[j], d2 = dx * dx + dy * dy + 0.002f;
        if (d2 < 0.16f) { fx += ks * dx / d2; fy += ks * dy / d2; }
      }
      const float r2 = px_[k] * px_[k] + py_[k] * py_[k];  // il "posatoio": lo stormo non esce dal cielo
      fx -= kh * px_[k] * r2; fy -= kh * py_[k] * r2;
      // muro morbido: oltre 0,9 il cielo respinge con forza crescente (gli uccelli non toccano il bordo)
      if (px_[k] > 0.9f) fx -= 6.f * (px_[k] - 0.9f); else if (px_[k] < -0.9f) fx -= 6.f * (px_[k] + 0.9f);
      if (py_[k] > 0.9f) fy -= 6.f * (py_[k] - 0.9f); else if (py_[k] < -0.9f) fy -= 6.f * (py_[k] + 0.9f);
      // vagare individuale (rumore lisciato)
      wx_[k] += 0.08f * ((rand01() - 0.5f) * 6.f - wx_[k]); wy_[k] += 0.08f * ((rand01() - 0.5f) * 6.f - wy_[k]);
      fx += wx_[k] * (0.5f + 0.9f * (1.f - coh_)); fy += wy_[k] * (0.5f + 0.9f * (1.f - coh_));
      fx += gx_ * 0.6f; fy += gy_ * 0.6f;                    // lo stormo intero vira lentamente nel cielo
      if (atk_ > 0.01f) {                                  // fuga dal falco
        const float dx = px_[k] - ax_, dy = py_[k] - ay_, d2 = dx * dx + dy * dy + 0.01f;
        fx += atk_ * 1.6f * dx / d2; fy += atk_ * 1.6f * dy / d2;
      }
      vx_[k] += fx * dt; vy_[k] += fy * dt;
      float s2 = vx_[k] * vx_[k] + vy_[k] * vy_[k];
      if (s2 > vmax * vmax) { const float g = vmax / sqrtf_(s2); vx_[k] *= g; vy_[k] *= g; }
      else if (s2 < vmin * vmin && s2 > 1e-8f) { const float g = vmin / sqrtf_(s2); vx_[k] *= g; vy_[k] *= g; }
      px_[k] += vx_[k] * dt; py_[k] += vy_[k] * dt;
      px_[k] = clampf(px_[k], -1.5f, 1.5f); py_[k] = clampf(py_[k], -1.5f, 1.5f);
      // dal volo al suono: ritardo (con rampa per campione), panorama, scurimento
      dT_[k] = target(k); dS_[k] = clampf((dT_[k] - d_[k]) * (1.f / 32.f), -0.03f, 0.03f);   // max ~50 cents
      const float pan = clampf(px_[k] * width_ * 1.3f, -1.f, 1.f);
      glT_[k] = sqrtf_(1.f - pan); grT_[k] = sqrtf_(1.f + pan);
      const float far = clampf(0.5f + 0.5f * py_[k], 0.f, 1.f);
      const float fc = 14000.f * pow2(-far * tone_ * 3.f);
      const float x = 6.2831853f * fc / kSR; tc_[k] = x / (1.f + x);
    }
  }
  float target(int k) const {                              // 6 ms + profondita' (fino a ~22 ms) + piccolo scarto per uccello
    static const float off[MAXB] = {0.f, 0.7f, 1.3f, 0.4f, 1.9f, 1.0f, 2.4f, 0.2f};
    const float y = clampf(0.5f + 0.5f * py_[k], 0.f, 1.f);
    return (6.f + off[k] + y * depth_ * 22.f) * 48.f;
  }
  inline float readH(float d) const {
    const float rp = (float)w_ - d; int32_t i = (int32_t)rp; if (rp < 0.f && (float)i != rp) --i; const float f = rp - (float)i;
    const float xm1 = buf_[(uint32_t)(i - 1) & MASK], x0 = buf_[(uint32_t)i & MASK], x1 = buf_[(uint32_t)(i + 1) & MASK], x2 = buf_[(uint32_t)(i + 2) & MASK];
    const float c1 = 0.5f * (x1 - xm1), c2 = xm1 - 2.5f * x0 + 2.f * x1 - 0.5f * x2, c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
    return ((c3 * f + c2) * f + c1) * f + x0;
  }
  static inline float sinp(float x) { const float q0 = x * 0.159154943f; float q = q0 - (float)(int32_t)q0; if (q < 0.f) q += 1.f; const float t = q < 0.5f ? q * 4.f - 1.f : 3.f - q * 4.f; return t * (1.5f - 0.5f * t * t); }
  static inline float clampf(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }
  static inline float lim(float y) { const float a = y < 0.f ? -y : y; if (a <= 0.8f) return y; const float e = (a - 0.8f) * 5.f; const float l = 0.8f + 0.2f * e / (1.f + e); return y < 0.f ? -l : l; }
  static inline float sqrtf_(float x) { if (x <= 0.f) return 0.f; float y = x > 1.f ? x : 1.f; for (int k = 0; k < 6; ++k) y = 0.5f * (y + x / y); return y; }
  static inline float pow2(float x) { const bool ng = x < 0.f; if (ng) x = -x; const int32_t ip = (int32_t)x; const float f = x - ip;
    const float r = (float)(1u << (ip > 30 ? 30 : ip)) * (1.f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * 0.00961813f)))); return ng ? 1.f / r : r; }
  inline float rand01() { rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5; return (rng_ >> 8) * (1.f / 16777216.f); }

  float *buf_ = nullptr; uint32_t w_ = 0, tick_ = 0, rng_ = 1, ppq_ = 0; bool pendAtk_ = false;
  float speed_, depth_, coh_, pred_, width_, tone_, fb_, mix_, bpm_, fbs_, atk_, ax_, ay_; int nb_; uint8_t sync_;
  float px_[MAXB], py_[MAXB], vx_[MAXB], vy_[MAXB], wx_[MAXB], wy_[MAXB], d_[MAXB], dT_[MAXB], dS_[MAXB], lp_[MAXB];
  float gl_[MAXB], gr_[MAXB], glT_[MAXB], grT_[MAXB], tc_[MAXB];
  float gph_ = 0.f, gph2_ = 1.f, gx_ = 0.f, gy_ = 0.f;
};
