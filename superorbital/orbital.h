#pragma once
/*
 *  SUPERORBITAL v0.2 - estensione di ORBITAL14 (motore originale dell'utente: 4 bande complementari
 *  panoramizzate + 2 campane EQ animate, modi DEEP/ORBIT/GALAXY/QUANT/EQ WAVE/EQ RAND) con due modi nuovi:
 *  DOPPLER (4 satelliti in orbita: panorama, distanza, brillantezza ed effetto Doppler per banda)
 *  ROTARY  (altoparlante rotante: tamburo < 850 Hz e tromba > 850 Hz, inerzia dei rotori).
 *  v0.2: XPHASER (4..16 stadi, risonanza fino a 0,95), XFLANGE / XJET (feedback +/-0,97): le due campane EQ
 *  inseguono lo sweep e il segnale trattato gira nelle 4 bande orbitali.
 *  Funzioni trigonometriche ed esponenziali approssimate (nessuna dipendenza da libm).
 */
#include <stdint.h>
class Orbital {
 public:
  enum { P_SPEED = 0, P_SPREAD, P_MODE, P_EQAMT, P_MOTION, P_DEPTH, P_SWEEP, P_Q, P_PEAK, P_MIX, P_NUM };
  enum { M_DEEP = 0, M_ORBIT, M_GALAXY, M_QUANT, M_EQWAVE, M_EQRAND, M_DOPPLER, M_ROTARY, M_XPHASER, M_XFLANGE, M_XJET, M_NUM };
  enum { DN = 1024, DM = DN - 1 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kFloats = 8 * DN;            // 4 bande x 2 canali, per il Doppler

  void init(float *mem) {
    for (int k = 0; k < 8; ++k) dl_[k] = mem + k * DN;
    // valori iniziali come ORBITAL14 (SPEED 400, SPREAD 900, EQ AMT 850, MOTION 700, MIX 1000, DEPTH 450, SWEEP 800, Q 650, PEAK 850)
    p_[P_SPEED] = 0.4f; p_[P_SPREAD] = 0.9f; p_[P_EQAMT] = 0.85f; p_[P_MOTION] = 0.7f; p_[P_DEPTH] = 0.45f;
    p_[P_SWEEP] = 0.8f; p_[P_Q] = 0.65f; p_[P_PEAK] = 0.85f; p_[P_MIX] = 1.f; mode_ = M_ORBIT;
    for (int i = 0; i < P_NUM; ++i) sm_[i] = p_[i];
    clear();
  }
  void clear() {
    for (int c = 0; c < 2; ++c) for (int k = 0; k < 3; ++k) lp_[c][k] = 0.f;
    phase_ = 0.f; samples_ = 0; for (int k = 0; k < NMUL; ++k) mph_[k] = 0.f;
    for (int k = 0; k < 4; ++k) { pan_[k] = 0.f; eq_[k] = 1.f; rpos_[k] = 0.f; req_[k] = 0.f; tpos_[k] = 0.f; teq_[k] = 0.f; dly_[k] = dlyT_[k] = 6.f * 48.f; dark_[k] = 1.f; for (int c = 0; c < 2; ++c) blp_[k][c] = 0.f; }
    for (int k = 0; k < 8; ++k) for (uint32_t i = 0; i < DN; ++i) dl_[k][i] = 0.f;
    dw_ = 0; for (int b = 0; b < 2; ++b) { peak_[b] = Biquad(); updatePeak(peak_[b], b ? 1800.f : 450.f, 0.f, 1.f); }
    hornPh_ = drumPh_ = 0.f; hornHz_ = 0.8f; drumHz_ = 0.67f; rng_ = 0x53A41F21u;
  }
  void setParam(uint8_t id, int32_t v) {
    if (id == P_MODE) { mode_ = (uint8_t)(v < 0 ? 0 : (v >= M_NUM ? M_NUM - 1 : v)); return; }
    if (id == P_SPEED || id == P_SPREAD) { p_[id] = v * (1.f / 1023.f); return; }
    if (id < P_NUM) p_[id] = v * 0.001f;
  }
  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *names[] = {"DEEP", "ORBIT", "GALAXY", "QUANT", "EQ WAVE", "EQ RAND", "DOPPLER", "ROTARY", "XPHASER", "XFLANGE", "XJET"};
    if (id == P_MODE && v >= 0 && v < M_NUM) return names[v];
    return nullptr;
  }

  __attribute__((optimize("O2")))
  void process(const float *in, float *out, uint32_t frames) {
    const int mode = mode_;
    const bool dop = (mode == M_DOPPLER || mode == M_ROTARY);
    for (uint32_t n = 0; n < frames; ++n) {
      for (int i = 0; i < P_NUM; ++i) sm_[i] += (p_[i] - sm_[i]) * .002f;
      const float speed = sm_[P_SPEED], width = sm_[P_SPREAD], amount = sm_[P_EQAMT], motion = sm_[P_MOTION], mix = sm_[P_MIX], depth = sm_[P_DEPTH];
      const float sweep = sm_[P_SWEEP], resonance = sm_[P_Q], eqmix = sm_[P_PEAK];
      if ((samples_++ & 31u) == 0u) control(mode, speed, width, amount, motion, depth, sweep, resonance, eqmix);
      // ritardi del Doppler: inseguono il valore di controllo campione per campione (niente scalini)
      if (dop) for (int b = 0; b < 4; ++b) dly_[b] += (dlyT_[b] - dly_[b]) * 0.02f;
      float wetL = 0.f, wetR = 0.f;
      for (int c = 0; c < 2; ++c) {
        float x = in[2 * n + c];
        // EQ prima della scomposizione spettrale (come ORBITAL14)
        float filtered = runPeak(peak_[0], c, x);
        filtered = runPeak(peak_[1], c, filtered);
        x = x * (1.f - eqmix) + filtered * eqmix;
        if (mode >= M_XPHASER) x = xfx(c, x, mode);
        static const float coeff[3] = {.0233f, .1053f, .3594f};
        for (int k = 0; k < 3; ++k) lp_[c][k] += coeff[k] * (x - lp_[c][k]);
        float band[4] = {lp_[c][0], lp_[c][1] - lp_[c][0], lp_[c][2] - lp_[c][1], x - lp_[c][2]};
        for (int b = 0; b < 4; ++b) {
          float v = band[b];
          if (dop) {                                   // satellite: ritardo Doppler + scurimento con la distanza
            float *d = dl_[b * 2 + c]; d[dw_] = v;
            v = readH(d, dly_[b]);
            blp_[b][c] += (v - blp_[b][c]) * dark_[b]; v = blp_[b][c];
          }
          v *= eq_[b];
          const float left = .5f * (1.f - pan_[b]), right = 1.f - left;
          wetL += v * (.8f * left + (c == 0 ? .2f : 0.f));
          wetR += v * (.8f * right + (c == 1 ? .2f : 0.f));
        }
      }
      if (dop || mode >= M_XFLANGE) dw_ = (dw_ + 1) & DM;
      wetL *= .78f; wetR *= .78f;
      const float l = in[2 * n] * (1.f - mix) + wetL * mix;
      const float r = in[2 * n + 1] * (1.f - mix) + wetR * mix;
      out[2 * n] = lim(l / (1.f + .18f * absf(l)));               // limitatore di ORBITAL14 + morbido sopra 0,8 (mai oltre 1)
      out[2 * n + 1] = lim(r / (1.f + .18f * absf(r)));
    }
  }

 private:
  struct Biquad { float b0 = 1.f, b1 = 0.f, b2 = 0.f, a1 = 0.f, a2 = 0.f, z1[2] = {0.f, 0.f}, z2[2] = {0.f, 0.f}; };
  static void updatePeak(Biquad &f, float hz, float db, float q) {
    // coseno come 1 - 2 sin^2(w/2): esatto anche per angoli piccoli (le campane basse a Q alto ne dipendono)
    const float w = 6.28318530718f * hz / 48000.f, sh = fsin(w * 0.5f), cs = 1.f - 2.f * sh * sh, sn = fsin(w);
    const float A = fpow2(db * .025f * 3.3219281f), alpha = sn / (2.f * q);      // 10^(db/40)
    const float inv = 1.f / (1.f + alpha / A);
    f.b0 = (1.f + alpha * A) * inv; f.b1 = (-2.f * cs) * inv; f.b2 = (1.f - alpha * A) * inv;
    f.a1 = (-2.f * cs) * inv; f.a2 = (1.f - alpha / A) * inv;
  }
  static inline float runPeak(Biquad &f, int c, float x) {
    const float y = f.b0 * x + f.z1[c];
    f.z1[c] = f.b1 * x - f.a1 * y + f.z2[c]; f.z2[c] = f.b2 * x - f.a2 * y;
    return y;
  }

  // ---------- controllo (ogni 32 campioni) ----------
  void control(int mode, float speed, float width, float amount, float motion, float depth, float sweep, float resonance, float eqmix) {
    const float hz = .012f * fpow2(speed * 11.3807f);       // 0,012 .. 32 Hz (come ORBITAL14: 0,012 * 2666,67^speed)
    const float dph = hz * 32.f / 48000.f;
    phase_ += dph;
    // fasi continue per i multipli non interi (in ORBITAL14 saltavano a ogni giro della fase principale)
    static const float mul[NMUL] = {0.35f, 0.27f, 0.7f, 1.25f, 0.8f, 1.6f};
    for (int k = 0; k < NMUL; ++k) { mph_[k] += dph * mul[k]; if (mph_[k] >= 1.f) mph_[k] -= 1.f; }
    if (phase_ >= 1.f) { phase_ -= (float)(int32_t)phase_;
      for (int b = 0; b < 4; ++b) { tpos_[b] = rnd(); teq_[b] = rnd(); }
      pw_[0] = rnd(); pw_[1] = rnd(); }
    for (int b = 0; b < 4; ++b) { rpos_[b] += .09f * (tpos_[b] - rpos_[b]); req_[b] += .09f * (teq_[b] - req_[b]); }
    const float t = 6.28318530718f * phase_;
    const float t035 = 6.28318530718f * mph_[0], t027 = 6.28318530718f * mph_[1], t07 = 6.28318530718f * mph_[2];
    if (mode >= M_XPHASER) { xcontrol(mode, t, width, amount, motion, depth, sweep, resonance, eqmix); return; }
    // --- due campane EQ animate (identiche a ORBITAL14) ---
    const float q = .65f + 17.35f * resonance * resonance;
    for (int k = 0; k < 2; ++k) {
      const float offset = k ? 2.0943951f : 0.f;
      float wave = fsin(t + offset);
      if (mode == M_EQRAND) wave = .65f * req_[k] + .35f * fsin(t + offset);
      else if (mode == M_QUANT) wave = .5f * req_[k] + .5f * fsin(2.f * t + offset);
      const float center = (k ? 2400.f : 480.f) * fpow2(5.f * sweep * wave);
      float envelope = fcos(t + (k ? 1.6f : 0.f));
      if (mode == M_EQRAND) envelope = req_[k];
      if (mode == M_GALAXY) envelope = fcos(t * 2.f + offset);
      const float db = 24.f * amount * eqmix * envelope;
      updatePeak(peak_[k], clipf(center, 55.f, 12500.f), db, q);
    }
    if (mode == M_DOPPLER) { doppler(hz, width, amount, motion, depth); return; }
    if (mode == M_ROTARY) { rotary(speed, width, amount, motion, depth); return; }
    // --- modi originali di ORBITAL14 ---
    for (int b = 0; b < 4; ++b) {
      const float a = t + b * 1.570796327f;
      float pos = 0.f, env = 0.f;
      if (mode == M_DEEP) { pos = fsin(t035 + b * 1.57f); env = fcos(t027 + b * 1.5f); }
      else if (mode == M_ORBIT) { pos = fsin(t * (b & 1 ? -1.f : 1.f) + b * .87f); env = fcos(t + b * 1.57f); }
      else if (mode == M_GALAXY) { const float expand = .5f + .5f * fsin(t07); pos = (b & 1 ? -1.f : 1.f) * expand * fsin(t + b * .51f); env = fcos(t * 2.f + b * .9f); }
      else if (mode == M_QUANT) { pos = .8f * rpos_[b] + .2f * fsin(t * 3.f + b); env = .8f * req_[b] + .2f * fsin(t * 2.f + b); }
      else if (mode == M_EQWAVE) { pos = fsin(a); env = fsin(a); }
      else { pos = rpos_[b]; env = req_[b]; }
      const float anchor = b == 0 ? .25f : (b == 1 ? .65f : 1.f);
      pan_[b] = clipf(pos * width * motion * anchor, -1.f, 1.f);
      const float db = (mode >= M_EQWAVE ? 18.f : 12.f) * amount * env;
      eq_[b] = fpow2(db * .05f * 3.3219281f);                  // 10^(db/20)
      if (mode < M_EQWAVE) eq_[b] *= (1.f - .55f * depth * (.5f + .5f * fcos(t07 + b)));
    }
  }
  // DOPPLER: ogni banda e' un satellite con la sua orbita (velocita' e verso diversi)
  void doppler(float hz, float width, float amount, float motion, float depth) {
    static const float ratio[4] = {1.f, -1.25f, 0.8f, 1.6f};
    static const float off[4] = {0.f, 1.9f, 3.6f, 5.1f};
    for (int b = 0; b < 4; ++b) {
      // fase continua di ogni satellite (verso negativo per la banda 1)
      static const int8_t idx[4] = {-1, 3, 4, 5};
      const float ph = idx[b] < 0 ? phase_ : mph_[idx[b]];
      const float th = 6.28318530718f * ph * (ratio[b] < 0.f ? -1.f : 1.f) + off[b];
      const float x = fcos(th), z = fsin(th);                   // x: sinistra/destra, z: vicino (-) / lontano (+)
      const float anchor = b == 0 ? .35f : (b == 1 ? .7f : 1.f);   // i bassi orbitano piu' stretti (come ORBITAL14)
      pan_[b] = clipf(x * width * anchor, -1.f, 1.f);
      const float dist = 1.f + z * motion * 0.75f;              // 0,25 .. 1,75
      float g = 1.f / dist; g = g > 2.5f ? 2.5f : g;
      eq_[b] = g * (1.f + amount * 0.3f * (1.f - dist));        // vicino: piu' presente
      // vicino = brillante, lontano = scuro (passa basso per banda)
      const float fc = 16000.f * fpow2(-z * motion * 2.2f - 0.6f);
      const float xx = 6.2831853f * (fc > 18000.f ? 18000.f : fc) / kSR; dark_[b] = xx / (1.f + xx);
      // Doppler: il ritardo segue la distanza (avvicinandosi cala -> l'altezza sale).
      // L'ampiezza e' normalizzata sulla velocita' dell'orbita: DEPTH = scostamento massimo fino a ~+/-100 cents a qualunque SPEED.
      const float w = 6.2831853f * hz * (ratio[b] < 0.f ? -ratio[b] : ratio[b]);
      float amp = depth * 0.06f / (w > 0.01f ? w : 0.01f);         // secondi
      if (amp > 0.0045f) amp = 0.0045f;
      dlyT_[b] = (0.0062f + z * amp) * kSR;
    }
  }
  // ROTARY: tamburo (bande 0-1) e tromba (bande 2-3), con inerzia; SPEED < 50% = lento, >= 50% = veloce
  void rotary(float speed, float width, float amount, float motion, float depth) {
    const bool veloce = speed >= 0.5f;
    const float hornT = veloce ? 6.8f : 0.8f, drumT = veloce ? 5.9f : 0.67f;
    const float dt = 32.f / kSR;
    hornHz_ += (hornT - hornHz_) * (dt / 0.8f);               // la tromba accelera in ~1 s
    drumHz_ += (drumT - drumHz_) * (dt / 3.0f);               // il tamburo in ~3 s
    hornPh_ += hornHz_ * dt; if (hornPh_ >= 1.f) hornPh_ -= 1.f;
    drumPh_ += drumHz_ * dt; if (drumPh_ >= 1.f) drumPh_ -= 1.f;
    for (int b = 0; b < 4; ++b) {
      const bool horn = b >= 2;
      const float th = 6.28318530718f * (horn ? hornPh_ : drumPh_) + (b & 1 ? 0.35f : 0.f);
      const float s = fsin(th), c = fcos(th);
      pan_[b] = clipf(s * width * (horn ? 0.9f : 0.6f), -1.f, 1.f);
      // modulazione d'ampiezza: la bocca della tromba (o del tamburo) che guarda verso di te
      const float am = 1.f - motion * (horn ? 0.45f : 0.3f) * (0.5f - 0.5f * c);
      eq_[b] = am * (1.f + amount * 0.15f * c) * 2.f;             // compensazione: tamburo e tromba rendono circa 6 dB in meno
      const float xx = 6.2831853f * (horn ? 9000.f + 6000.f * c : 5000.f + 2000.f * c) / kSR; dark_[b] = xx / (1.f + xx);
      dlyT_[b] = (horn ? 1.2f + depth * 0.9f * s : 1.0f + depth * 0.35f * s) * 48.f;   // Doppler della rotazione
    }
  }

  // ---------- XPHASER / XFLANGE / XJET ----------
  // Parametri riusati: EQ AMT = risonanza, MOTION = escursione, DEPTH = stadi (XPHASER), SWEEP = centro / ritardo,
  // Q e PEAK EQ = selettivita' e quantita' delle campane che inseguono lo sweep, SPREAD = stereo e orbita.
  void xcontrol(int mode, float t, float width, float amount, float motion, float depth, float sweep, float resonance, float eqmix) {
    const float offR = 6.28318530718f * width * 0.5f;           // sfasamento destro fino a 180 gradi
    const float lL = fsin(t), lR = fsin(t + offR);
    const float q = .65f + 17.35f * resonance * resonance;
    const float bdb = 24.f * eqmix;                             // campane fino a +24 dB
    if (mode == M_XPHASER) {
      xst_ = 4 + 2 * (int32_t)(depth * 6.f + 0.5f);             // 4, 6, 8, 10, 12, 14, 16 stadi
      xfb_ = amount * 0.95f;
      const float fc0 = 90.f * fpow2(sweep * 5.6f);             // centro 90 Hz .. ~4,4 kHz
      const float oct = motion * 3.f;                           // fino a +/- 3 ottave
      float fL = fc0 * fpow2(oct * lL), fR = fc0 * fpow2(oct * lR);
      fL = clipf(fL, 30.f, 16000.f); fR = clipf(fR, 30.f, 16000.f);
      const float aL = coefA(fL), aR = coefA(fR);
      xas_[0] = (aL - xa_[0]) * (1.f / 32.f); xas_[1] = (aR - xa_[1]) * (1.f / 32.f);   // interpolazione per campione
      // le campane inseguono lo sweep: sul punto a 90 gradi e sul picco successivo (formante mobile)
      const float fm = (fL + fR) * 0.5f;
      updatePeak(peak_[0], clipf(fm, 55.f, 12500.f), bdb, q);
      updatePeak(peak_[1], clipf(fm * 2.2f, 55.f, 12500.f), bdb * 0.8f, q);
    } else {
      const bool jet = (mode == M_XJET);
      xfb_ = amount * 0.97f * (jet ? -1.f : 1.f);
      const float man = (0.3f + sweep * 9.7f) * 0.001f;         // ritardo di partenza 0,3 .. 10 ms
      const float dep = motion * 0.008f;                        // escursione fino a 8 ms
      const float dL = man + dep * (0.5f + 0.5f * lL), dR = man + dep * (0.5f + 0.5f * lR);
      xdT_[0] = dL * kSR; xdT_[1] = dR * kSR;
      // campane sui picchi del pettine: 1/d (feedback positivo) o 1/(2d) e armoniche dispari (negativo)
      const float dm = (dL + dR) * 0.5f;
      const float f1 = jet ? 0.5f / dm : 1.f / dm;
      updatePeak(peak_[0], clipf(f1, 55.f, 12500.f), bdb, q);
      updatePeak(peak_[1], clipf(f1 * (jet ? 3.f : 2.f), 55.f, 12500.f), bdb * 0.8f, q);
    }
    // il segnale trattato gira nelle 4 bande con le traiettorie di ORBIT
    for (int b = 0; b < 4; ++b) {
      const float pos = fsin(t * (b & 1 ? -1.f : 1.f) + b * .87f);
      const float anchor = b == 0 ? .25f : (b == 1 ? .65f : 1.f);
      pan_[b] = clipf(pos * width * 0.85f * anchor, -1.f, 1.f);
      eq_[b] = 1.f;
    }
  }
  __attribute__((optimize("O2")))
  float xfx(int c, float x, int mode) {
    if (mode == M_XPHASER) {
      xa_[c] += xas_[c];
      const float base = xa_[c];
      static const float sp[16] = {1.f, 1.07f, .94f, 1.12f, .9f, 1.05f, .97f, 1.1f, .92f, 1.03f, .96f, 1.08f, .95f, 1.06f, .98f, 1.04f};
      const float ab = 1.f - (base < 0.f ? -base : base);
      float v = x + soft(xfbs_[c]) * xfb_;                      // risonanza con saturazione morbida nell'anello
      for (int k = 0; k < xst_; ++k) {
        const float a = base + (sp[k] - 1.f) * 0.15f * ab;
        const float y = a * v + xs_[c][k];
        xs_[c][k] = v - a * y;
        v = y;
      }
      xfbs_[c] = v;
      return (x + v) * 0.5f * xmk();                            // dry + sfasato: le tacche
    }
    // flanger: Hermite sulla linea del canale, feedback +/- con saturazione
    xd_[c] += (xdT_[c] - xd_[c]) * 0.01f;
    float *d = dl_[c];
    const float y = readH(d, xd_[c] < 2.f ? 2.f : xd_[c]);
    d[dw_] = soft(x + y * xfb_);
    return (x + y) * 0.5f * xmk();
  }
  inline float xmk() const { const float f = xfb_ < 0.f ? -xfb_ : xfb_; return 1.6f / (1.f + f * 0.55f); }   // livello vicino al bypass anche con risonanza alta
  static inline float coefA(float f) {                          // passa-tutto del primo ordine: (t-1)/(t+1), t = tan(pi f/fs)
    const float x = 3.14159265f * f / kSR, x2 = x * x;
    const float t = x * (1.f + x2 * (0.3333333f + x2 * (0.1333333f + x2 * 0.0539683f)));
    return (t - 1.f) / (t + 1.f);
  }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }

  // ---------- utilita' ----------
  inline float readH(const float *b, float d) const {
    const float rp = (float)dw_ - d; int32_t i = (int32_t)rp; if (rp < 0.f && (float)i != rp) --i; const float f = rp - (float)i;
    const float xm1 = b[(uint32_t)(i - 1) & DM], x0 = b[(uint32_t)i & DM], x1 = b[(uint32_t)(i + 1) & DM], x2 = b[(uint32_t)(i + 2) & DM];
    const float c1 = 0.5f * (x1 - xm1), c2 = xm1 - 2.5f * x0 + 2.f * x1 - 0.5f * x2, c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
    return ((c3 * f + c2) * f + c1) * f + x0;
  }
  static inline float absf(float x) { return x < 0.f ? -x : x; }
  static inline float lim(float y) { const float a = y < 0.f ? -y : y; if (a <= 0.8f) return y; const float e = (a - 0.8f) * 5.f; const float l = 0.8f + 0.2f * e / (1.f + e); return y < 0.f ? -l : l; }
  static inline float clipf(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
  // seno con riduzione dell'argomento e polinomio di 9 grado
  static inline float fsin(float x) {
    const float inv2pi = 0.159154943f; float q = x * inv2pi; q -= (float)(int32_t)(q >= 0.f ? q + 0.5f : q - 0.5f);   // q in [-0.5, 0.5]
    float y = q * 6.28318530718f;                                 // [-pi, pi]
    if (y > 1.5707963f) y = 3.14159265f - y; else if (y < -1.5707963f) y = -3.14159265f - y;
    const float y2 = y * y;
    return y * (1.f - y2 * (0.16666667f - y2 * (0.0083333333f - y2 * (0.00019841270f - y2 * 0.0000027557319f))));   // errore ~4e-6
  }
  static inline float fcos(float x) { return fsin(x + 1.5707963f); }
  static inline float fpow2(float x) {                            // 2^x per |x| < 30
    const bool neg = x < 0.f; if (neg) x = -x;
    const int32_t ip = (int32_t)x; const float f = x - (float)ip;
    const float r = (float)(1u << (ip > 30 ? 30 : ip)) * (1.f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * 0.00961813f))));
    return neg ? 1.f / r : r;
  }
  inline float rnd() { rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5; return (float)(rng_ & 65535u) * (2.f / 65535.f) - 1.f; }

  float p_[P_NUM] = {0}, sm_[P_NUM] = {0}; uint8_t mode_ = M_ORBIT;
  float lp_[2][3], phase_, pan_[4], eq_[4], rpos_[4], req_[4], tpos_[4], teq_[4], pw_[2] = {0.f, 0.f};
  float dly_[4], dlyT_[4], dark_[4], blp_[4][2];
  enum { NMUL = 6 }; float mph_[NMUL] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
  float *dl_[8]; uint32_t dw_ = 0, samples_ = 0, rng_ = 0x53A41F21u;
  float hornPh_, drumPh_, hornHz_, drumHz_;
  Biquad peak_[2];
  // stato dei modi X
  int32_t xst_ = 8; float xfb_ = 0.f, xa_[2] = {0.f, 0.f}, xas_[2] = {0.f, 0.f}, xs_[2][16] = {{0}}, xfbs_[2] = {0.f, 0.f}, xd_[2] = {96.f, 96.f}, xdT_[2] = {96.f, 96.f};
};
