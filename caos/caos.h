#pragma once
/*
 *  CAOS v0.2 - modulazione guidata dall'attrattore di Lorenz (v0.2: escursioni piene, velocita' su una corsa
 *  molto piu' ampia, flanger fino al jet, DTYPE RING e DRIVE).
 *  dx = s(y - x), dy = x(r - z) - y, dz = xy - b z  (s = 10, b = 8/3).
 *  CHAOS sposta r da 350 (orbita periodica, quasi un LFO) a 28 (caos classico): x -> panorama (le due "ali"),
 *  y -> filtro, z -> ritardo breve (flanger, chorus, vibrato). Variabili normalizzate sulla scala dell'attrattore.
 *  Nessuna dipendenza da libm.
 */
#include <stdint.h>
class Caos {
 public:
  enum { P_SPEED = 0, P_CHAOS, P_DEST, P_DEPTH, P_DTYPE, P_FB, P_CUTOFF, P_RESO, P_WIDTH, P_MIX, P_NUM };
  enum { D_TUTTO = 0, D_RITARDO, D_FILTRO, D_PAN };
  enum { T_FLANGE = 0, T_CHORUS, T_VIBRATO, T_RING, T_DRIVE };
  enum { N = 2048, MASK = N - 1 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kFloats = 2 * N;

  void init(float *mem) {
    bL_ = mem; bR_ = mem + N;
    speed_ = 0.5f; chaos_ = 0.75f; dest_ = D_TUTTO; depth_ = 0.8f; dtype_ = T_CHORUS; fb_ = 0.4f; cutoff_ = 0.55f; reso_ = 0.5f; width_ = 0.9f; mix_ = 0.85f;
    clear();
  }
  void clear() {
    for (uint32_t i = 0; i < kFloats; ++i) bL_[i] = 0.f;
    w_ = 0; tick_ = 0; x_ = 1.f; y_ = 1.f; z_ = 20.f; nx_ = ny_ = nz_ = 0.f; sx_ = sy_ = sz_ = 0.f;
    d_ = 600.f; lpL_ = lpR_ = bpL_ = bpR_ = 0.f; fbL_ = fbR_ = 0.f; gL_ = gR_ = 1.f; fg_ = 0.3f; fq_ = 1.f;
  }
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_SPEED:  speed_ = v * (1.f / 1023.f); break;
      case P_CHAOS:  chaos_ = v * (1.f / 1023.f); break;
      case P_DEST:   dest_ = (uint8_t)clampi(v, 0, 3); break;
      case P_DEPTH:  depth_ = v * 0.01f; break;
      case P_DTYPE:  dtype_ = (uint8_t)clampi(v, 0, 4); break;
      case P_FB:     fb_ = v * 0.01f; break;
      case P_CUTOFF: cutoff_ = v * 0.01f; break;
      case P_RESO:   reso_ = v * 0.01f; break;
      case P_WIDTH:  width_ = v * 0.01f; break;
      case P_MIX:    mix_ = v * 0.01f; break;
      default: break;
    }
  }
  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *ds[] = {"TUTTO", "RITARDO", "FILTRO", "PAN"};
    static const char *ts[] = {"FLANGE", "CHORUS", "VIBRATO", "RING", "DRIVE"};
    if (id == P_DEST && v >= 0 && v < 4) return ds[v];
    if (id == P_DTYPE && v >= 0 && v < 5) return ts[v];
    return nullptr;
  }

  void process(const float *in, float *out, uint32_t frames) {
    const bool useD = dest_ == D_TUTTO || dest_ == D_RITARDO, useF = dest_ == D_TUTTO || dest_ == D_FILTRO, useP = dest_ == D_TUTTO || dest_ == D_PAN;
    for (uint32_t n = 0; n < frames; ++n) {
      if ((tick_++ & 15u) == 0u) control();
      // segnali di Lorenz lisciati campione per campione
      sx_ += (nx_ - sx_) * 0.06f; sy_ += (ny_ - sy_) * 0.06f; sz_ += (nz_ - sz_) * 0.06f;
      const float xl = in[2 * n], xr = in[2 * n + 1];
      float l = xl, r = xr;
      if (useD && dtype_ == T_RING) {                                 // ring modulator: portante 30 Hz .. ~3 kHz guidata da z
        const float f = 30.f * pow2(3.3f + 3.3f * sz_ * depth_);
        rph_ += f / kSR; if (rph_ >= 1.f) rph_ -= 1.f;
        const float cl = sinp(rph_), cr = sinp(rph_ + 0.25f * width_ >= 1.f ? rph_ + 0.25f * width_ - 1.f : rph_ + 0.25f * width_);
        const float dp = 0.5f + 0.5f * depth_, rg = 1.f + 0.5f * dp; l = xl * (1.f - dp + dp * cl) * rg; r = xr * (1.f - dp + dp * cr) * rg;
      } else if (useD && dtype_ == T_DRIVE) {                         // saturazione guidata dal caos (z = spinta, x = asimmetria)
        const float dr = 1.f + 14.f * depth_ * (0.5f + 0.5f * sz_), bs = 0.3f * depth_ * sx_, mk = (1.f + depth_) / (1.f + 9.f * depth_ * (0.5f + 0.5f * sz_));
        l = (soft(xl * dr + bs) - soft(bs)) * mk * 0.62f; r = (soft(xr * dr - bs) - soft(-bs)) * mk * 0.62f;
      } else if (useD) {                                              // ritardo guidato da z
        bL_[w_] = soft(xl + fbL_); bR_[w_] = soft(xr + fbR_);
        static const float base[3] = {0.3f, 8.f, 5.f}, exc[3] = {6.f, 20.f, 6.f};   // ms: FLANGE 0,3..6,3; CHORUS 8..28; VIBRATO 5..11
        const float dT = (base[dtype_] + exc[dtype_] * depth_ * (0.5f + 0.5f * sz_)) * 48.f;
        d_ += (dT - d_) * 0.004f;
        const float dl = d_ * (1.f - 0.15f * width_ * sx_), dr = d_ * (1.f + 0.15f * width_ * sx_);
        const float yl = rd(bL_, dl), yr = rd(bR_, dr);
        const float f = dtype_ == T_VIBRATO ? 0.f : fb_ * (dtype_ == T_FLANGE ? -0.92f : 0.6f);   // FLANGE: feedback negativo (jet)
        fbL_ = yl * f; fbR_ = yr * f;
        if (dtype_ == T_VIBRATO) { l = yl * 1.3f; r = yr * 1.3f; } else { l = (xl + yl) * 0.78f; r = (xr + yr) * 0.78f; }
        w_ = (w_ + 1) & MASK;
      }
      if (useF) {                                                     // filtro guidato da y
        l = svf(lpL_, bpL_, l); r = svf(lpR_, bpR_, r);
      }
      if (useP) {                                                     // panorama guidato da x (le due ali)
        const float p = clampf(sx_ * width_ * depth_ * 1.6f, -1.f, 1.f);
        const float tl = p > 0.f ? 1.f - p : 1.f, tr = p < 0.f ? 1.f + p : 1.f;
        gL_ += (tl - gL_) * 0.01f; gR_ += (tr - gR_) * 0.01f;
        const float m = 0.5f * (l + r) * 0.5f * width_;
        l = (l * (1.f - 0.5f * width_) + m) * gL_ * 1.15f; r = (r * (1.f - 0.5f * width_) + m) * gR_ * 1.15f;
      }
      out[2 * n] = lim(xl * (1.f - mix_) + l * mix_);
      out[2 * n + 1] = lim(xr * (1.f - mix_) + r * mix_);
    }
  }
  // per le prove
  float nx() const { return nx_; } float ny() const { return ny_; } float nz() const { return nz_; }

 private:
  void control() {
    // r: da 350 (orbita periodica) a 28 (caos classico), con una curva esponenziale
    const float rr = 28.f * pow2((1.f - chaos_) * 3.644f);
    const float s = 10.f, b = 2.6666667f;
    // velocita': da ~0,02 a ~2 giri al secondo circa (scala dei tempi dell'attrattore)
    const float tscale = 0.08f * pow2(speed_ * 8.5f);
    const float h = tscale * 16.f / kSR;                               // passo di integrazione per blocco di controllo
    const int sub = 8; const float hs = h / sub;                   // piu' sotto-passi: stabile anche alla velocita' massima
    for (int k = 0; k < sub; ++k) {                                    // Runge-Kutta del secondo ordine (punto medio)
      const float dx1 = s * (y_ - x_), dy1 = x_ * (rr - z_) - y_, dz1 = x_ * y_ - b * z_;
      const float xm = x_ + 0.5f * hs * dx1, ym = y_ + 0.5f * hs * dy1, zm = z_ + 0.5f * hs * dz1;
      x_ += hs * s * (ym - xm); y_ += hs * (xm * (rr - zm) - ym); z_ += hs * (xm * ym - b * zm);
    }
    // sicurezza: se esce di scala (non dovrebbe) si riparte vicino a un'ala
    if (!(x_ == x_) || x_ > 1000.f || x_ < -1000.f || z_ > 2000.f || z_ < -100.f) { x_ = 1.f; y_ = 1.f; z_ = rr * 0.5f; }
    // normalizzazione sulla scala dell'attrattore: centri delle ali a +/- sqrt(b(r-1)), z attorno a r-1
    const float c = sqrtf_(b * (rr - 1.f));
    nx_ = clampf(x_ / (c * 1.6f), -1.f, 1.f);
    ny_ = clampf(y_ / (c * 2.2f), -1.f, 1.f);
    nz_ = clampf((z_ - (rr - 1.f)) / (rr * 0.55f), -1.f, 1.f);
    // filtro: centro CUTOFF (100 Hz .. 12 kHz), escursione con y e DEPTH
    float e = 6.9f * cutoff_ + 6.f * depth_ * sy_; e = e < 0.f ? 0.f : (e > 7.1f ? 7.1f : e);
    float g = 3.14159265f * (100.f * pow2(e)) / kSR; if (g > 0.95f) g = 0.95f;
    fg_ = g / (1.f + g); fq_ = 1.5f - 1.44f * reso_;
  }
  inline float svf(float &lp, float &bp, float x) {
    const float g = fg_, k = fq_;
    const float hp = (x - (k + g) * bp - lp) / (1.f + g * (k + g)); const float b1 = g * hp + bp; bp = g * hp + b1; const float l1 = g * b1 + lp; lp = g * b1 + l1; return soft(l1);
  }
  inline float rd(const float *b, float d) const {                    // Hermite
    const float rp = (float)w_ - d; int32_t i = (int32_t)rp; if (rp < 0.f && (float)i != rp) --i; const float f = rp - (float)i;
    const float xm1 = b[(uint32_t)(i - 1) & MASK], x0 = b[(uint32_t)i & MASK], x1 = b[(uint32_t)(i + 1) & MASK], x2 = b[(uint32_t)(i + 2) & MASK];
    const float c1 = 0.5f * (x1 - xm1), c2 = xm1 - 2.5f * x0 + 2.f * x1 - 0.5f * x2, c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
    return ((c3 * f + c2) * f + c1) * f + x0;
  }
  static inline int clampi(int32_t v, int a, int b) { return v < a ? a : (v > b ? b : (int)v); }
  static inline float clampf(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }
  static inline float sinp(float p) { const float t = p < 0.5f ? p * 4.f - 1.f : 3.f - p * 4.f; return -t * (1.5f - 0.5f * t * t); }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }
  static inline float lim(float y) { const float a = y < 0.f ? -y : y; if (a <= 0.8f) return y; const float e = (a - 0.8f) * 5.f; const float l = 0.8f + 0.2f * e / (1.f + e); return y < 0.f ? -l : l; }
  static inline float sqrtf_(float x) { if (x <= 0.f) return 0.f; float y = x > 1.f ? x : 1.f; for (int k = 0; k < 8; ++k) y = 0.5f * (y + x / y); return y; }
  static inline float pow2(float x) { const bool ng = x < 0.f; if (ng) x = -x; const int32_t ip = (int32_t)x; const float f = x - ip;
    const float r = (float)(1u << (ip > 30 ? 30 : ip)) * (1.f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * 0.00961813f)))); return ng ? 1.f / r : r; }

  float *bL_ = nullptr, *bR_ = nullptr; uint32_t w_ = 0, tick_ = 0;
  float speed_, chaos_, depth_, fb_, cutoff_, reso_, width_, mix_; uint8_t dest_, dtype_;
  float rph_ = 0.f;
  float x_, y_, z_, nx_, ny_, nz_, sx_, sy_, sz_, d_, lpL_, lpR_, bpL_, bpR_, fbL_, fbR_, gL_, gR_, fg_, fq_;
};
