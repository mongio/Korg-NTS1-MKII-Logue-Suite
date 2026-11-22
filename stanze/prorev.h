#pragma once
/*
 *  prorev.h - motore di STANZE: riverbero a due reti stereo indipendenti (2 linee per canale), scritto da zero.
 *  Ricostruzione a scatola nera del carattere di quattro riverberi logue per Prologue (MIST, HAZE, THEATRE, CATHEDRAL):
 *  nessun codice degli originali e' stato letto o tradotto; i valori vengono dalle misure in emulazione.
 *
 *  Modalita':  taglia S (ritardi base) / L (ritardi doppi)   x   filtro d'uscita HP (chiaro) / LP (scuro)
 *    NEBBIA = S + HP (MIST),  FOSCHIA = L + HP (HAZE),  TEATRO = S + LP (THEATRE),  CATTEDRALE = L + LP (CATHEDRAL)
 *
 *  Struttura per canale: ingresso (proprio canale + 0,32 dell'altro) -> 2 linee modulate, ognuna con un breve diffusore,
 *  mescolate da una rotazione ortogonale e retroazionate con guadagno g(TIME); uscita = somma delle linee
 *  + una piccola parte diretta; poi filtro di secondo ordine HP o LP (taglio da DEPTH), poi mix a potenza costante.
 */
#include <stdint.h>

class ProRev {
 public:
  enum { P_TIME = 0, P_DEPTH, P_MIX, P_MODE, P_MOD, P_SIZE, P_NUM };
  enum { M_NEBBIA = 0, M_FOSCHIA, M_TEATRO, M_CATTEDRALE };
  // ritardi base misurati (campioni a 48 kHz): sinistra 20,73 / 35,10 ms, destra 27,48 / 46,19 ms
  static constexpr float kBase[4] = {991.f, 1682.f, 1315.f, 2212.f};   // ritoccati sulle prime riflessioni misurate (entrambe le taglie)
  static constexpr int kMaxSize = 2;
  enum { LINE = 16384, LMASK = LINE - 1, AP = 64, APMASK = AP - 1 };      // 4 x 64 KB in SDRAM (SIZE fino a x2 nella taglia L)
  static constexpr uint32_t kFloats = 4 * LINE + 4 * AP;

  void init(float *mem, int mode = M_NEBBIA) {
    for (int k = 0; k < 4; ++k) { line_[k] = mem + k * LINE; ap_[k] = mem + 4 * LINE + k * AP; }
    mode_ = mode & 3; big_ = (mode_ & 1); lp_ = (mode_ >= 2);
    { const float w = 6.28318530718f * 90.f / 48000.f, sn = fsin(w), sh = fsin(0.5f * w), cs = 1.f - 2.f * sh * sh;   // 90 Hz fisso
      const float al = sn * 0.70710678f, a0 = 1.f / (1.f + al); h0_ = (1.f + cs) * 0.5f * a0; ha1_ = -2.f * cs * a0; ha2_ = (1.f - al) * a0; }
    // correzione di livello per variante (misurata con rumore, filtro aperto): MIST, HAZE, THEATRE, CATHEDRAL
    setTrim();
    time_ = 0.5f; depth_ = 0.5f; mix_ = 0.5f; mod_ = 0.5f; size_ = 0.5f;
    clear(); update();
  }
  void clear() {
    for (uint32_t i = 0; i < kFloats; ++i) line_[0][i] = 0.f;
    w_ = 0; apw_ = 0; for (int k = 0; k < 4; ++k) fb_[k] = 0.f;
    l1_ = 112.f / 360.f; l2_ = 283.f / 360.f;              // fasi iniziali misurate
    for (int c = 0; c < 2; ++c) { z1_[c] = z2_[c] = 0.f; hz1_[c] = hz2_[c] = 0.f; }
    mixs_ = mix_; fenv_ = ftarget_ = 0.f; fhold_ = 0; for (int k = 0; k < 4; ++k) { apz_[k] = 0.f; blp_[k] = 0.f; } sl_[0] = sl_[1] = 0.f;
  }
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_TIME:  time_ = v * (1.f / 1023.f); break;
      case P_DEPTH: depth_ = v * (1.f / 1023.f); break;
      case P_MIX:   mix_ = (v + 1000) * (1.f / 2000.f); break;     // -1000..1000 -> 0..1 (SHIFT-DEPTH dell'originale)
      case P_MODE:  { const int m = (v < 0 ? 0 : (v > 3 ? 3 : v)); if (m != mode_ && m != pend_) startSwitch(m); } return;
      case P_MOD:   mod_ = v * 0.01f; break;                       // 50% = come l'originale
      case P_SIZE:  size_ = v * 0.01f; break;                      // 50% = come l'originale
      default: break;
    }
    update();
  }

  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *modes[] = {"NEBBIA", "FOSCHIA", "TEATRO", "CATTEDRALE"};
    if (id == P_MODE && v >= 0 && v < 4) return modes[v];
    return nullptr;
  }
  int mode() const { return mode_; }

  void process(const float *in, float *out, uint32_t frames) {
    if (phase_ == PH_CLEAR) clearStep();                         // azzeramento della rete a pezzi, un blocco alla volta
    for (uint32_t n = 0; n < frames; ++n) {
      const float xl = in[2 * n], xr = in[2 * n + 1];
      // "wow" globale (misurato sull'originale): TUTTI i ritardi si allungano e accorciano insieme, in proporzione
      // alla loro lunghezza, con due LFO (0,32 e 0,16 Hz). La variazione si accumula a ogni giro della coda:
      // la coda oscilla d'intonazione e le risonanze si "spalmano" (risposta liscia anche sulle note gravi).
      l1_ += 0.32f / 48000.f; if (l1_ >= 1.f) l1_ -= 1.f;
      l2_ += 0.16f / 48000.f; if (l2_ >= 1.f) l2_ -= 1.f;
      if ((n & 7u) == 0u) eps_ = wowA_ * fsin(6.28318530718f * l1_) + wowB_ * fsin(6.28318530718f * l2_);
      float y[4];
      for (int k = 0; k < 4; ++k) y[k] = readA(k, dly_[k] * (1.f + eps_));
      // canale sinistro: linee 0-1, destro: linee 2-3 (reti indipendenti, solo l'ingresso si mescola)
      const float inL = xl + 0.32f * xr, inR = xr + 0.32f * xl;
      float gl = gL_, gr = gR_;
      if (frozen_) {
        // controllo automatico del livello della coda congelata (come il FREEZE di RICORDO):
        // il riferimento segue la coda mentre suoni e per 0,3 s dopo; poi la retroazione si corregge di pochissimo
        const float e = y[0] * y[0] + y[1] * y[1] + y[2] * y[2] + y[3] * y[3];
        fenv_ += (e - fenv_) * (1.f / 24000.f);
        const float xa = (xl < 0.f ? -xl : xl) + (xr < 0.f ? -xr : xr);
        if (xa > 1e-4f) fhold_ = 14400; else if (fhold_ > 0) --fhold_;
        if (fhold_ > 0 || ftarget_ <= 0.f) ftarget_ = fenv_;
        else if (fenv_ > 1e-12f) {
          float c = 1.f + 0.0004f * log2f_(ftarget_ / fenv_);
          c = c < 0.9997f ? 0.9997f : (c > 1.0006f ? 1.0006f : c);
          gl = gr = c;
        }
      }
      // miscelazione ortogonale tra le due linee di ogni rete: [c s; s -c]. Con c piccolo l'energia passa soprattutto
      // da una linea all'altra (come nell'originale: gli echi "stessa linea due volte" sono deboli)
      float rc[4] = {kC * y[0] + kS * y[1], kS * y[0] - kC * y[1], kC * y[2] + kS * y[3], kS * y[2] - kC * y[3]};
      if (bass_ != 0.f) for (int k = 0; k < 4; ++k) { blp_[k] += (rc[k] - blp_[k]) * kBassC; rc[k] += bass_ * blp_[k]; }   // bassi piu' lunghi
      float v[4] = {inL * kIn + rc[0] * gl, inL * kIn + rc[1] * gl, inR * kIn + rc[2] * gr, inR * kIn + rc[3] * gr};
      if (phase_ == PH_CLEAR) { for (int k = 0; k < 4; ++k) v[k] = 0.f; }
      for (int k = 0; k < 4; ++k) { v[k] = limL(diffuse(k, v[k])); line_[k][w_] = v[k]; }     // rete di sicurezza: morbida sopra 6, mai oltre 8
      w_ = (w_ + 1) & LMASK; apw_ = (apw_ + 1) & APMASK;
      // uscita: somma delle linee + piccola parte diretta, poi filtro HP/LP di secondo ordine
      // parte diretta: solo il proprio canale (nella famiglia scura esce un campione dopo, come nell'originale)
      const float dl = lp_ ? pxl_ : xl, dr = lp_ ? pxr_ : xr; pxl_ = xl; pxr_ = xr;
      float nl = (y[0] + y[1]) * kOut * trim_, nr = (y[2] + y[3]) * kOut * trim_;
      // scaffale sui bassi della coda: -3 dB sotto ~250 Hz (colore misurato dell'originale)
      sl_[0] += (nl - sl_[0]) * kShelfC; sl_[1] += (nr - sl_[1]) * kShelfC; nl -= kShelfK * sl_[0]; nr -= kShelfK * sl_[1];
      float wl = (nl + dl * kDirect) * wetg_, wr = (nr + dr * kDirect) * wetg_;
      wl = biq(0, wl); wr = biq(1, wr);
      // mix a potenza costante: dry = sqrt(1-x), wet = sqrt(x) (come SHIFT-DEPTH dell'originale)
      mixs_ += (mix_ - mixs_) * 0.002f;
      const float gd = sqrtf_(1.f - mixs_), gw = sqrtf_(mixs_);
      // cambio di modalita': dissolvenza della parte wet
      if (phase_ == PH_OUT) { fadeg_ -= fstep_; if (fadeg_ <= 0.f) { fadeg_ = 0.f; endFadeOut(); } }
      else if (phase_ == PH_IN) { fadeg_ += fstep_; if (fadeg_ >= 1.f) { fadeg_ = 1.f; phase_ = PH_NONE; } }
      wl *= fadeg_; wr *= fadeg_;
      out[2 * n] = lim(xl * gd + wl * gw);
      out[2 * n + 1] = lim(xr * gd + wr * gw);
    }
  }

  // --- cambio di modalita' ---
  // stessa taglia (NEBBIA<->TEATRO, FOSCHIA<->CATTEDRALE): la coda continua, cambia solo il filtro (10 ms fuori + 10 ms dentro)
  // taglia diversa: 20 ms fuori, azzeramento della rete distribuito su piu' blocchi, 20 ms dentro
  enum { PH_NONE = 0, PH_OUT, PH_CLEAR, PH_IN };
  void startSwitch(int m) {
    pend_ = m; same_ = ((m & 1) == (mode_ & 1));
    fstep_ = 1.f / (same_ ? 480.f : 960.f);
    if (phase_ == PH_NONE || phase_ == PH_IN) phase_ = PH_OUT;
  }
  void endFadeOut() {
    if (same_) { applyMode(pend_); for (int c = 0; c < 2; ++c) z1_[c] = z2_[c] = 0.f; phase_ = PH_IN; }
    else { phase_ = PH_CLEAR; clr_ = 0; }
  }
  void clearStep() {                                            // 8192 campioni per blocco: la rete intera in ~10 blocchi
    const uint32_t n = 4 * LINE + 4 * AP, stepN = 8192;
    uint32_t e = clr_ + stepN; if (e > n) e = n;
    for (uint32_t i = clr_; i < e; ++i) line_[0][i] = 0.f;
    clr_ = e;
    if (clr_ >= n) {
      for (int k = 0; k < 4; ++k) { apz_[k] = 0.f; blp_[k] = 0.f; }
      sl_[0] = sl_[1] = 0.f; for (int c = 0; c < 2; ++c) z1_[c] = z2_[c] = 0.f;
      fenv_ = ftarget_ = 0.f; fhold_ = 0;
      applyMode(pend_); fstep_ = 1.f / 960.f; phase_ = PH_IN;
    }
  }
  void applyMode(int m) { mode_ = m & 3; big_ = (mode_ & 1); lp_ = (mode_ >= 2); setTrim(); update(); pend_ = -1; }
  void setTrim() {   // correzione di livello per modalita' (misurata con rumore, filtro aperto)
    static const float trim[4] = {0.6032f, 0.5891f, 0.6172f, 0.5957f};   // NEBBIA, FOSCHIA, TEATRO, CATTEDRALE
    trim_ = trim[(lp_ ? 2 : 0) + (big_ ? 1 : 0)];
  }

  // --- tabelle ricavate dalle misure (11 punti, TIME e DEPTH da 0 a 1) ---
  // RT60 della taglia S in secondi; la taglia L ha ritardi doppi e quindi, a parita' di g, coda doppia
  static constexpr float kRT[11] = {3.19f, 3.36f, 3.82f, 4.49f, 5.41f, 6.70f, 8.63f, 11.83f, 18.2f, 37.24f, 0.f};   // misurati (taglia S); 0 = congelata
  // filtro d'uscita di secondo ordine misurato (f0 in Hz, Q) per DEPTH = 0, 0.1 .. 1; nella famiglia chiara il wet viene alzato (dB) a DEPTH alto
  static constexpr float kHPf[11] = {100.f, 152.f, 232.f, 353.f, 538.f, 825.f, 1271.f, 1979.f, 3154.f, 5102.f, 7767.f};
  static constexpr float kHPq[11] = {1.01f, 1.01f, 1.00f, 0.99f, 0.99f, 0.98f, 0.96f, 0.94f, 0.89f, 0.85f, 0.76f};
  static constexpr float kHPg[11] = {0.0f, 0.0f, 0.1f, 0.1f, 0.2f, 0.4f, 0.6f, 1.1f, 2.0f, 3.4f, 4.4f};
  static constexpr float kLPf[11] = {90.f, 146.f, 233.f, 362.f, 554.f, 851.f, 1338.f, 2126.f, 3459.f, 5991.f, 12712.f};
  static constexpr float kLPg[11] = {1.9f, 0.7f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};   // compensazione a DEPTH basso
  static constexpr float kLPq[11] = {0.72f, 0.86f, 0.92f, 0.96f, 0.97f, 0.95f, 0.89f, 0.85f, 0.82f, 0.78f, 0.75f};
  // guadagni di retroazione tarati per riprodurre i decadimenti misurati (taglia S e taglia L), TIME = 0, 0.1 .. 1
  static constexpr float kGS[11] = {0.929223f, 0.932387f, 0.940592f, 0.949959f, 0.959134f, 0.967534f, 0.975281f, 0.982366f, 0.988839f, 0.994800f, 1.f};
  static constexpr float kGL[11] = {0.936627f, 0.940531f, 0.947837f, 0.955434f, 0.962820f, 0.969801f, 0.976461f, 0.982844f, 0.988993f, 0.994945f, 1.f};
  float kIn = 0.5f, kOut = 0.5f, kDirect = 0.0916f;     // calibrati sul livello misurato
  float gOverride = -1.f;                              // solo per la taratura (negativo = spento)
  float kC = 0.70710678f, kS = 0.70710678f;            // miscelazione (tarata sotto)
  float trim_ = 1.f, pxl_ = 0.f, pxr_ = 0.f, gL_ = 0.9f, gR_ = 0.9f, apz_[4] = {0.f, 0.f, 0.f, 0.f};
  int mode_ = 0, pend_ = -1, phase_ = PH_NONE; bool same_ = true; float fadeg_ = 1.f, fstep_ = 1.f / 480.f; uint32_t clr_ = 0;
  float wetg_ = 1.f, sl_[2] = {0.f, 0.f};
  static constexpr float kShelfC = 0.0322f, kShelfK = 0.292f;   // ~250 Hz, -3 dB
  float bass_ = 0.f, blp_[4] = {0.f, 0.f, 0.f, 0.f}; static constexpr float kBassC = 0.062f;   // ~500 Hz
  bool frozen_ = false; float fenv_ = 0.f, ftarget_ = 0.f; int32_t fhold_ = 0;

 private:
  void update() {
    const float sz = (big_ ? 2.f : 1.f) * pow2(( size_ - 0.5f) * 2.f);          // SIZE: x0,5 .. x2 attorno all'originale
    for (int k = 0; k < 4; ++k) { float d = kBase[k] * sz; if (d > LINE - 1200) d = LINE - 1200; dly_[k] = d; }
    // retroazione dal tempo di decadimento voluto: g = 10^(-3 * D / (RT * fs)), D = ritardo medio della rete
    // retroazione dalla tabella tarata: interpolazione su log(1-g) (coda che cresce in modo naturale),
    // ultimo tratto lineare fino a g = 1 (coda congelata a TIME al massimo)
    g_ = gtab(big_ ? kGL : kGS, time_);
    // SIZE diverso da 50%: si mantiene lo stesso tempo di decadimento (g compensato sul nuovo ritardo)
    if (sz != (big_ ? 2.f : 1.f) && g_ < 1.f) g_ = pow2(log2f_(g_) * sz / (big_ ? 2.f : 1.f));
    if (gOverride >= 0.f) g_ = gOverride;
    // ogni rete ha la sua retroazione: la sinistra (ritardi piu' corti) dura il 93% della destra, come nell'originale
    frozen_ = (g_ >= 1.f);
    if (frozen_) { gL_ = gR_ = 1.f; }                            // congelata: il livello lo tiene il controllo automatico (process)
    else { gL_ = pow2(log2f_(g_) * 0.933f); gR_ = pow2(log2f_(g_) * 1.071f); }
    // sotto ~500 Hz la coda dura di piu' (misurato. Taglia S: x1,0 a TIME 0, ~x1,3 a meta' corsa; taglia L: ~x1,1)
    bass_ = 0.f;
    if (!frozen_) {
      static const float brS[11] = {1.0f, 1.3f, 1.62f, 1.8f, 1.8f, 1.8f, 1.65f, 1.45f, 1.3f, 1.15f, 1.0f};   // tarati sull'effetto ottenuto nella rete
      static const float brL[11] = {1.0f, 1.06f, 1.12f, 1.16f, 1.16f, 1.16f, 1.13f, 1.1f, 1.06f, 1.03f, 1.0f};
      const float *br = big_ ? brL : brS;
      float p = time_ * 10.f; if (p > 10.f) p = 10.f; const int i = (int)p; const float f = p - i;
      const float r = (i >= 10) ? 1.f : br[i] + (br[i + 1] - br[i]) * f;
      const float gm = 0.5f * (gL_ + gR_);
      bass_ = pow2(log2f_(gm) / r) / gm - 1.f;                 // guadagno in piu' sui bassi a ogni giro
    }
    // modulazione: escursione del ritardo (campioni), calibrata per ~20 cents come l'originale
    wowA_ = kWowA * mod_ * 2.f; wowB_ = kWowB * mod_ * 2.f;                          // MOD 50% = come l'originale
    // filtro d'uscita
    // DEPTH a destra = piu' riverbero in tutte le modalita' (v0.2): nella famiglia chiara la corsa del passa alto e' invertita
    const float dd = lp_ ? depth_ : 1.f - depth_;
    const float fc = lp_ ? tab(kLPf, dd) : tab(kHPf, dd);
    const float q = lp_ ? lin(kLPq, dd) : lin(kHPq, dd);
    setBiquad(fc, q, lp_);
    wetg_ = pow2(lin(lp_ ? kLPg : kHPg, dd) * 0.16609640f);       // dB -> lineare
  }
  float kWowA = 0.00223f, kWowB = 0.00210f;            // ampiezze misurate (0,32 e 0,16 Hz)
  float wowA_ = 0.00223f, wowB_ = 0.0021f, eps_ = 0.f;
  inline float diffuse(int k, float x) {                 // diffusore breve (allpass di ~7 campioni, doppio nella taglia L)
    static const int len[4] = {7, 6, 7, 5};
    const int L = len[k] * (big_ ? 2 : 1);
    float *b = ap_[k]; const float d = b[(apw_ - L) & APMASK];
    const float vv = x + 0.5f * d; b[apw_] = vv; return d - 0.5f * vv;
  }
  // lettura con interpolazione passa-tutto (Thiran del primo ordine): sposta il ritardo senza togliere energia
  // a nessuna frequenza (l'Hermite smussava gli acuti a ogni giro). La frazione resta in [0,5 , 1,5) per stabilita'.
  inline float readA(int k, float d) {
    const float* b = line_[k];
    int32_t di = (int32_t)d; float fr = d - (float)di;          // ritardo = di + fr
    if (fr < 0.5f) { fr += 1.f; --di; }
    const float eta = (1.f - fr) / (1.f + fr);
    const float a = b[(w_ - di) & LMASK], bb = b[(w_ - di - 1) & LMASK];
    const float y = eta * a + bb - eta * apz_[k];
    apz_[k] = y; return y;
  }
  inline float readL(const float *b, float d) const {    // lettura interpolata (Hermite)
    const float rp = (float)w_ - d; int32_t i = (int32_t)rp; if (rp < 0.f && (float)i != rp) --i; const float f = rp - (float)i;
    const float xm1 = b[(uint32_t)(i - 1) & LMASK], x0 = b[(uint32_t)i & LMASK], x1 = b[(uint32_t)(i + 1) & LMASK], x2 = b[(uint32_t)(i + 2) & LMASK];
    const float c1 = 0.5f * (x1 - xm1), c2 = xm1 - 2.5f * x0 + 2.f * x1 - 0.5f * x2, c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
    return ((c3 * f + c2) * f + c1) * f + x0;
  }
  void setBiquad(float fc, float q, bool lowpass) {      // secondo ordine con risonanza misurata
    const float w = 6.28318530718f * fc / 48000.f, sn = fsin(w), sh = fsin(0.5f * w), cs = 1.f - 2.f * sh * sh;
    const float alpha = sn / (2.f * q), a0 = 1.f / (1.f + alpha);
    if (lowpass) { b0_ = (1.f - cs) * 0.5f * a0; b1_ = (1.f - cs) * a0; b2_ = b0_; }
    else { b0_ = (1.f + cs) * 0.5f * a0; b1_ = -(1.f + cs) * a0; b2_ = b0_; }
    a1_ = -2.f * cs * a0; a2_ = (1.f - alpha) * a0;
  }
  inline float hpf(int c, float x) {                     // passa alto Butterworth fisso, 90 Hz
    const float y = h0_ * x + hz1_[c]; hz1_[c] = -2.f * h0_ * x - ha1_ * y + hz2_[c]; hz2_[c] = h0_ * x - ha2_ * y; return y;
  }
  inline float biq(int c, float x) { const float y = b0_ * x + z1_[c]; z1_[c] = b1_ * x - a1_ * y + z2_[c]; z2_[c] = b2_ * x - a2_ * y; return y; }
  static inline float tab(const float *t, float x) { float p = x * 10.f; if (p < 0.f) p = 0.f; if (p > 10.f) p = 10.f; int i = (int)p; if (i >= 10) return t[10]; const float f = p - i;
    if (t[i + 1] <= 0.f) return f < 0.999f ? t[i] / (1.f - f) : 0.f;      // verso il congelamento: la coda tende all'infinito
    return t[i] * pow2(f * log2f_(t[i + 1] / t[i])); }                    // interpolazione esponenziale tra i punti
  static inline float gtab(const float *t, float x) {
    float p = x * 10.f; if (p < 0.f) p = 0.f; if (p >= 10.f) return t[10];
    const int i = (int)p; const float f = p - i;
    if (i == 9) return t[9] + (1.f - t[9]) * f;                                      // verso il congelamento
    const float a = log2f_(1.f - t[i]), b = log2f_(1.f - t[i + 1]);
    return 1.f - pow2(a + (b - a) * f);
  }
  static inline float limL(float y) { const float a = y < 0.f ? -y : y; if (a <= 6.f) return y; const float e = (a - 6.f) * 0.5f; const float l = 6.f + 2.f * e / (1.f + e); return y < 0.f ? -l : l; }
  static inline float lim(float y) { const float a = y < 0.f ? -y : y; if (a <= 0.8f) return y; const float e = (a - 0.8f) * 5.f; const float l = 0.8f + 0.2f * e / (1.f + e); return y < 0.f ? -l : l; }
  static inline float lin(const float *t, float x) { float p = x * 10.f; if (p < 0.f) p = 0.f; if (p >= 10.f) return t[10]; const int i = (int)p; return t[i] + (t[i + 1] - t[i]) * (p - i); }
  static inline float wrap(float p) { return p >= 1.f ? p - 1.f : p; }
  static inline float sinp(float ph) { float x = ph < 0.5f ? ph * 4.f - 1.f : 3.f - ph * 4.f; return x * (1.5f - 0.5f * x * x); }
  static inline float fsin(float x) { const float q0 = x * 0.159154943f; float q = q0 - (float)(int32_t)(q0 >= 0.f ? q0 + 0.5f : q0 - 0.5f); float y = q * 6.28318530718f;
    if (y > 1.5707963f) y = 3.14159265f - y; else if (y < -1.5707963f) y = -3.14159265f - y; const float y2 = y * y;
    return y * (1.f - y2 * (0.16666667f - y2 * (0.0083333333f - y2 * (0.00019841270f - y2 * 0.0000027557319f)))); }
  static inline float pow2(float x) { const bool ng = x < 0.f; if (ng) x = -x; const int32_t ip = (int32_t)x; const float f = x - ip;
    const float r = (float)(1u << (ip > 30 ? 30 : ip)) * (1.f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * 0.00961813f)))); return ng ? 1.f / r : r; }
  static inline float pow10(float x) { return pow2(x * 3.3219281f); }
  static inline float log2f_(float x) {                     // log2 con la serie di atanh: errore < 1e-5
    union { float f; uint32_t i; } u = {x}; const float e = (float)(int32_t)((u.i >> 23) & 255) - 127.f; u.i = (u.i & 0x007FFFFF) | 0x3F800000;
    const float m = u.f, y = (m - 1.f) / (m + 1.f), y2 = y * y;
    return e + 2.8853901f * y * (1.f + y2 * (0.33333333f + y2 * (0.2f + y2 * (0.14285714f + y2 * 0.11111111f))));
  }
  static inline float sqrtf_(float x) { if (x <= 0.f) return 0.f; float y = x > 0.25f ? x : 0.5f; for (int k = 0; k < 5; ++k) y = 0.5f * (y + x / y); return y; }

  float *line_[4], *ap_[4]; uint32_t w_ = 0, apw_ = 0; int big_ = 0; bool lp_ = false;
  float time_, depth_, mix_, mod_, size_, mixs_, g_ = 0.9f, dly_[4], modA_, modB_, fb_[4], l1_, l2_;
  float b0_ = 1.f, b1_ = 0.f, b2_ = 0.f, a1_ = 0.f, a2_ = 0.f, z1_[2], z2_[2];
  float h0_ = 1.f, ha1_ = 0.f, ha2_ = 0.f, hz1_[2] = {0.f, 0.f}, hz2_[2] = {0.f, 0.f};
};
