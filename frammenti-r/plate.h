#pragma once
/*
 *  plate.h - riverbero a piastra (architettura di Dattorro, 1997), riscalato a 48 kHz,
 *  con SHIMMER (pitch shifter all'ottava sopra dentro la vasca) e FREEZE. Nessuna dipendenza da libm.
 */
#include <stdint.h>

class Plate {
 public:
  // lunghezze originali (29,761 kHz) x 1,6128 -> 48 kHz
  enum { I1 = 229, I2 = 173, I3 = 611, I4 = 447,
         A_MAP = 1083 + 32, A_D1 = 7182, A_AP = 2902, A_D2 = 6000,
         B_MAP = 1464 + 32, B_D1 = 6801, B_AP = 4283, B_D2 = 5101,
         SH = 4096, SHM = SH - 1 };
  static constexpr uint32_t kFloats = I1 + I2 + I3 + I4 + A_MAP + A_D1 + A_AP + A_D2 + B_MAP + B_D1 + B_AP + B_D2 + SH;

  void init(float *mem) {
    float *p = mem;
    auto take = [&](Line &l, uint32_t n) { l.b = p; l.n = n; l.w = 0; p += n; };
    take(i1_, I1); take(i2_, I2); take(i3_, I3); take(i4_, I4);
    take(am_, A_MAP); take(ad1_, A_D1); take(aap_, A_AP); take(ad2_, A_D2);
    take(bm_, B_MAP); take(bd1_, B_D1); take(bap_, B_AP); take(bd2_, B_D2);
    sh_ = p; clear();
  }
  void clear() {
    Line *ls[] = {&i1_, &i2_, &i3_, &i4_, &am_, &ad1_, &aap_, &ad2_, &bm_, &bd1_, &bap_, &bd2_};
    for (Line *l : ls) { for (uint32_t k = 0; k < l->n; ++k) l->b[k] = 0.f; l->w = 0; }
    for (uint32_t k = 0; k < SH; ++k) sh_[k] = 0.f;
    bw_ = dA_ = dB_ = 0.f; fbA_ = fbB_ = 0.f; lfo_ = 0.f; shw_ = 0; shp_ = 0.f; shv_ = 0.f; shlp_ = 0.f;
  }

  // decay 0..0.97, damping 0..1, shimmer 0..1, freeze: decay = 1 e niente ingresso
  void process(float in, float &outL, float &outR, float decay, float damping, float shimmer, bool freeze) {
    const float dec = freeze ? 0.9995f : decay * (1.f - shimmer * 0.1f);   // shimmer alto: coda un po' piu' corta (stabilita')
    const float damp = freeze ? 0.f : damping;
    // ingresso: banda, 4 diffusori
    float x = freeze ? 0.f : in;
    bw_ += (x - bw_) * 0.85f; x = bw_;
    x = ap(i1_, x, 0.75f); x = ap(i2_, x, 0.75f); x = ap(i3_, x, 0.625f); x = ap(i4_, x, 0.625f);
    // shimmer: un'ottava sopra di cio' che gira nella vasca, reiniettato
    float shv = 0.f;
    if (shimmer > 0.001f && !freeze) {                // in FREEZE lo shimmer non aggiunge energia
      sh_[shw_] = fbA_ + fbB_;
      shp_ += 1.f / 2048.f; if (shp_ >= 1.f) shp_ -= 1.f;
      for (int h = 0; h < 2; ++h) {
        float q = shp_ + 0.5f * h; if (q >= 1.f) q -= 1.f;
        const float d = 2048.f * (1.f - q) + 2.f;
        float rp = (float)shw_ - d; int32_t ip = (int32_t)rp; if (rp < 0.f && (float)ip != rp) --ip; const float f = rp - (float)ip;
        const float a = sh_[(uint32_t)ip & SHM], b = sh_[(uint32_t)(ip + 1) & SHM];
        const float u = q * (1.f - q); const float s = 16.f * u / (5.f - 4.f * u);
        shv += (a + (b - a) * f) * s * s;
      }
      shw_ = (shw_ + 1) & SHM;
      shlp_ += (shv - shlp_) * 0.45f;                       // passa basso (~6 kHz): le ottave altissime non si accumulano
      shv = shlp_ * shimmer * 0.32f;
    }
    // modulazione lenta dei diffusori della vasca (toglie il suono metallico)
    lfo_ += 1.f / 48000.f; if (lfo_ >= 1.f) lfo_ -= 1.f;
    const float m = tri(lfo_) * 16.f, m2 = tri(lfo_ + 0.25f) * 16.f;
    // ramo A
    float a = x + fbB_ * dec + shv;
    a = modAp(am_, a, -0.7f, 1083.f + m);
    ad1_.write(a); a = ad1_.read(A_D1 - 1);
    dA_ += (a - dA_) * (1.f - damp); a = dA_ * dec;
    a = ap(aap_, a, 0.5f);
    ad2_.write(a); fbA_ = soft(ad2_.read(A_D2 - 1));
    // ramo B
    float b = x + fbA_ * dec - shv;
    b = modAp(bm_, b, -0.7f, 1464.f + m2);
    bd1_.write(b); b = bd1_.read(B_D1 - 1);
    dB_ += (b - dB_) * (1.f - damp); b = dB_ * dec;
    b = ap(bap_, b, 0.5f);
    bd2_.write(b); fbB_ = soft(bd2_.read(B_D2 - 1));
    // uscite (prese multiple, come nel progetto originale)
    outL = 0.6f * (bd1_.read(429) + bd1_.read(4797) - bap_.read(3085) + bd2_.read(3219) - ad1_.read(3210) - aap_.read(302) - ad2_.read(1719));
    outR = 0.6f * (ad1_.read(569) + ad1_.read(5868) - aap_.read(1984) + ad2_.read(4321) - bd1_.read(3329) - bap_.read(504) - bd2_.read(176));
    i1_.adv(); i2_.adv(); i3_.adv(); i4_.adv(); am_.adv(); ad1_.adv(); aap_.adv(); ad2_.adv(); bm_.adv(); bd1_.adv(); bap_.adv(); bd2_.adv();
  }

 private:
  struct Line {
    float *b; uint32_t n, w;
    inline void write(float v) { b[w] = v; }
    inline float read(uint32_t d) const { uint32_t i = w + n - d; if (i >= n) i -= n; return b[i]; }   // d < n
    inline void adv() { if (++w >= n) w = 0; }
  };
  static inline float ap(Line &l, float x, float g) {   // allpass di Schroeder sulla linea (ritardo = n - 1)
    const float d = l.read(l.n - 1); const float v = x + g * d; l.write(v); return d - g * v;
  }
  static inline float modAp(Line &l, float x, float g, float dly) {
    const int32_t di = (int32_t)dly; const float f = dly - (float)di;
    const float d0 = l.read(di), d1 = l.read(di + 1); const float d = d0 + (d1 - d0) * f;
    const float v = x + g * d; l.write(v); return d - g * v;
  }
  static inline float tri(float p) { if (p >= 1.f) p -= 1.f; return p < 0.5f ? 4.f * p - 1.f : 3.f - 4.f * p; }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }
  Line i1_, i2_, i3_, i4_, am_, ad1_, aap_, ad2_, bm_, bd1_, bap_, bd2_;
  float *sh_ = nullptr; uint32_t shw_ = 0; float shp_ = 0.f, shv_ = 0.f, shlp_ = 0.f;
  float bw_, dA_, dB_, fbA_, fbB_, lfo_;
};
