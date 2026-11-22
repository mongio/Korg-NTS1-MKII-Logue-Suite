#pragma once
/*
 *  RADIOLINA v0.3 - oscillatore: una radiolina che ogni tanto trasmette una frase cantata (due frasi, PHRASE)
 *  e, con CHORDS, un pad di accordi nella tonalita' dell'ultima nota suonata, che cambiano ogni 1-8 battute.
 *  Ispirato a GHOST RADIO (sintonia su una stazione, rumore tra le stazioni, deriva, eco fantasma),
 *  riscritto per una sola stazione e una sola frase, con la catena "radiolina" (banda stretta, risonanza, saturazione)
 *  e rumori di fondo (fruscio, crepitii, ronzio di rete, scariche). Nessuna dipendenza da libm.
 */
#include <stdint.h>
#include "phrases.h"

class Radiolina {
 public:
  enum { P_TUNE = 0, P_EVERY, P_SIGNAL, P_RADIO, P_NOISE, P_DRIFT, P_GHOST, P_KEY, P_PHRASE, P_CHORDS, P_NUM };
  enum { NV = 5 };                                               // voci del pad: basso + 4
  enum { F_A = 0, F_B, F_ALT, F_RANDOM };
  enum { K_AUTO = 0, K_TRIGGER, K_PITCH };
  static constexpr float kSR = 48000.f;
  enum { EN = 2048, EM = EN - 1 };

  void init() {
    tune_ = 0.5f; every_ = 0.35f; signal_ = 0.7f; radio_ = 0.6f; noise_ = 0.4f; drift_ = 0.2f; ghost_ = 0.15f; key_ = K_AUTO; phrase_ = F_ALT; cur_ = 0; alt_ = 0;
    rng_ = 0x2A51D0C3u; dial_ = 0.5f; level_ = 0.f; pos_ = 0.f; playing_ = false; wait_ = (int32_t)(0.4f * kSR); pitch_ = 1.f;
    tick_ = 0; hp1_ = hp2_ = lp1_ = lp2_ = 0.f; for (int k = 0; k < 2; ++k) pz1_[k] = pz2_[k] = 0.f;
    wph_ = humph_ = 0.f; crack_ = 0.f; burst_ = 0.f; hissLP_ = 0.f; fade_ = 0.f; ew_ = 0; edec_ = 0; for (int i = 0; i < EN; ++i) echo_[i] = 0;
    elfo_ = 0.f; wander1_ = 0.f; wander2_ = 0.f; vl_ = 0.f;
    chords_ = 0; rootPc_ = 9; bpm_ = 120.f; ccnt_ = 0; deg_ = 0; cenv_ = 0.f; duck_ = 1.f; plfo_ = 0.f; plp1_ = plp2_ = 0.f;
    for (int v = 0; v < NV; ++v) { vf_[v] = vt_[v] = 220.f; vn_[v] = 57 + v * 4; ph1_[v] = 0.13f * v; ph2_[v] = 0.37f * v; }
    voice(true);
    updateRadio();
  }
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_TUNE:   tune_ = v * (1.f / 1023.f); break;
      case P_EVERY:  every_ = v * (1.f / 1023.f); break;
      case P_SIGNAL: signal_ = v * 0.01f; break;
      case P_RADIO:  radio_ = v * 0.01f; updateRadio(); break;
      case P_NOISE:  noise_ = v * 0.01f; break;
      case P_DRIFT:  drift_ = v * 0.01f; break;
      case P_GHOST:  ghost_ = v * 0.01f; break;
      case P_KEY:    key_ = (uint8_t)(v < 0 ? 0 : (v > 2 ? 2 : v)); if (key_ != K_PITCH) pitch_ = 1.f; break;
      case P_PHRASE: phrase_ = (uint8_t)(v < 0 ? 0 : (v > 3 ? 3 : v)); break;
      case P_CHORDS: { const uint8_t c = (uint8_t)(v < 0 ? 0 : (v > 16 ? 16 : v)); const bool newMode = c && ((c - 1) >> 2) != (chords_ ? (chords_ - 1) >> 2 : -1); chords_ = c; if (newMode) voice(false); } break;
      default: break;
    }
  }
  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *keys[] = {"AUTO", "TRIGGER", "PITCH"};
    static const char *phr[] = {"A", "B", "ALT", "RANDOM"};
    if (id == P_KEY && v >= 0 && v < 3) return keys[v];
    if (id == P_PHRASE && v >= 0 && v < 4) return phr[v];
    static const char *chs[] = {"OFF", "MAJ 1BAR", "MAJ 2BAR", "MAJ 4BAR", "MAJ 8BAR", "MIN 1BAR", "MIN 2BAR", "MIN 4BAR", "MIN 8BAR",
                                "DOR 1BAR", "DOR 2BAR", "DOR 4BAR", "DOR 8BAR", "MIX 1BAR", "MIX 2BAR", "MIX 4BAR", "MIX 8BAR"};
    if (id == P_CHORDS && v >= 0 && v < 17) return chs[v];
    return nullptr;
  }
  void setTempo(float bpm) { bpm_ = bpm < 30.f ? 30.f : (bpm > 300.f ? 300.f : bpm); }
  void noteOn(uint8_t note) {
    if ((int)(note % 12) != rootPc_) { rootPc_ = note % 12; deg_ = 0; voice(false); ccnt_ = 0; }   // nuova tonalita': si riparte dal primo grado
    if (key_ == K_TRIGGER) start((int)note);
    else if (key_ == K_PITCH) pitch_ = pow2((float)((int)note - 60) * (1.f / 12.f));
  }

  void process(float *out, uint32_t frames) {
    for (uint32_t i = 0; i < frames; ++i) {
      if ((tick_++ & 31u) == 0u) control();
      // --- la frase: lettura mu-law interpolata ---
      float voice = 0.f;
      if (playing_) {
        const uint32_t ix = (uint32_t)pos_;
        if (ix + 1 < kPhraseLen[cur_]) {
          while (dn_ <= ix + 1) decodeNext();                    // decodifica ADPCM in avanti, fino al campione ix+1
          const float f = pos_ - (float)ix;
          voice = (s0_ + (s1_ - s0_) * f) * (1.f / 32768.f);
          pos_ += (kPhraseRate / kSR) * pitch_ * (1.f + wob_);
        } else { playing_ = false; wait_ = nextWait(); }
      } else if (key_ != K_TRIGGER) {
        if (--wait_ <= 0) start();
      }
      // --- eco fantasma (buffer decimato 4:1, ~140 ms, ritardo che ondeggia) ---
      float ghost = 0.f;
      if (ghost_ > 0.005f) {
        elfo_ += 0.21f / kSR; if (elfo_ >= 1.f) elfo_ -= 1.f;
        const float d = 1700.f + 60.f * sinp(elfo_);
        float rp = (float)ew_ - d; if (rp < 0.f) rp += (float)EN; const int32_t ri = (int32_t)rp; const float rf = rp - (float)ri;
        const float e0 = echo_[ri & EM] * (1.f / 32767.f), e1 = echo_[(ri + 1) & EM] * (1.f / 32767.f);
        ghost = e0 + (e1 - e0) * rf;
        if ((++edec_ & 3u) == 0u) { float wv = voice + ghost * 0.35f * ghost_; wv = wv > 1.f ? 1.f : (wv < -1.f ? -1.f : wv); echo_[ew_] = (int16_t)(wv * 32767.f); ew_ = (ew_ + 1) & EM; }
      }
      // --- ricezione: stazione e fischio di battimento fuori sintonia ---
      wph_ += whz_ * (1.f / kSR); if (wph_ >= 1.f) wph_ -= 1.f;
      const float whistle = sinp(wph_) * whl_;
      float prog = (voice + ghost * ghost_ * 0.8f + (chords_ ? pad() : 0.f)) * level_ * fade_ + whistle;
      // --- rumori di fondo ---
      const float n = noise();
      hissLP_ += 0.35f * (n - hissLP_);
      const float hiss = 0.55f * n + 0.45f * hissLP_;
      if (rand01() < crackP_) crack_ = (rand01() * 2.f - 1.f) * (0.4f + 0.6f * rand01());
      crack_ *= 0.86f;
      humph_ += 50.f / kSR; if (humph_ >= 1.f) humph_ -= 1.f;
      const float hum = sinp(humph_) * 0.6f + sinp(wrap(humph_ * 2.f)) * 0.3f + sinp(wrap(humph_ * 3.f)) * 0.15f;
      if ((tick_ & 8191u) == 0u && rand01() < 0.15f + 0.5f * noise_) burst_ = 0.6f + 0.4f * rand01();
      burst_ *= 0.9994f;
      float bg = hiss * hissA_ + crack_ * crackA_ + hum * humA_ + burst_ * n * 0.25f * noise_;
      // --- la radiolina: banda stretta, risonanza, saturazione ---
      float x = prog + bg;
      hp1_ += hpc_ * (x - hp1_); x -= hp1_; hp2_ += hpc_ * (x - hp2_); x -= hp2_;        // passa alto 2 poli
      lp1_ += lpc_ * (x - lp1_); lp2_ += lpc_ * (lp1_ - lp2_); x = lp2_;                // passa basso 2 poli
      const float pk = pb0_ * x + pz1_[0]; pz1_[0] = pb1_ * x - pa1_ * pk + pz2_[0]; pz2_[0] = pb2_ * x - pa2_ * pk; x = pk;   // risonanza
      x = soft(x * drive_) * makeup_;
      vl_ += 0.0005f * ((x < 0.f ? -x : x) - vl_);
      out[i] = x > 0.95f ? 0.95f : (x < -0.95f ? -0.95f : x);
    }
  }

 private:
  // sceglie la frase (A, B, alternate o a caso; in TRIGGER con ALT: tasti bianchi = A, neri = B) e riparte da capo
  // ---------- il pad di accordi ----------
  float pad() {
    float s = 0.f;
    for (int v = 0; v < NV; ++v) {
      const float f = vf_[v] * (1.f / kSR);
      ph1_[v] += f * 1.004f; if (ph1_[v] >= 1.f) ph1_[v] -= 1.f;      // due seghe leggermente scordate
      ph2_[v] += f * 0.996f; if (ph2_[v] >= 1.f) ph2_[v] -= 1.f;
      s += (ph1_[v] + ph2_[v] - 1.f) * (v == 0 ? 1.1f : 0.8f);
    }
    plp1_ += pc_ * (s - plp1_); plp2_ += pc_ * (plp1_ - plp2_);      // morbido: due poli
    return plp2_ * 0.11f * duck_ * (0.75f + 0.25f * cenv_);
  }
  void nextChord() {
    // progressione: dal grado attuale verso i gradi "naturali" (pesi su 16)
    static const uint8_t W[7][7] = {
      {1, 2, 1, 4, 4, 3, 1}, {1, 0, 1, 2, 7, 1, 4}, {1, 1, 0, 3, 2, 7, 2}, {4, 2, 1, 0, 6, 2, 1},
      {7, 1, 1, 3, 0, 3, 1}, {2, 4, 1, 5, 3, 0, 1}, {9, 1, 3, 1, 1, 1, 0}};
    int tot = 0; for (int k = 0; k < 7; ++k) tot += W[deg_][k];
    int r = (int)(crand() * (float)tot), k = 0;
    while (k < 6 && r >= W[deg_][k]) { r -= W[deg_][k]; ++k; }
    deg_ = k; voice(false); cenv_ = 0.f;
  }
  // accordo sul grado deg_ della scala, voci mosse verso le note piu' vicine
  void voice(bool init) {
    static const uint8_t SC[4][7] = {{0, 2, 4, 5, 7, 9, 11}, {0, 2, 3, 5, 7, 8, 10}, {0, 2, 3, 5, 7, 9, 10}, {0, 2, 4, 5, 7, 9, 10}};
    const int mode = chords_ ? (chords_ - 1) >> 2 : 0;
    int tones[4];
    for (int i = 0; i < 4; ++i) { const int st = deg_ + 2 * i; tones[i] = rootPc_ + SC[mode][st % 7] + 12 * (st / 7); }
    const bool seventh = crand() < 0.3f;
    int bass = 36 + (tones[0] % 12); if (bass < 40) bass += 12;      // basso tra Mi1 e Re#2
    vn_[0] = bass;
    // condotta delle voci: tra le 24 assegnazioni delle 4 note alle 4 voci alte, quella con il movimento totale minimo
    int pcs[4] = {tones[0] % 12, tones[1] % 12, tones[2] % 12, (seventh ? tones[3] : tones[0]) % 12};
    int bestSet[4] = {0, 0, 0, 0}, bestCost = 1 << 30;
    for (int a = 0; a < 4; ++a) for (int b = 0; b < 4; ++b) for (int c = 0; c < 4; ++c) {
      const int d = 6 - a - b - c; if (a == b || a == c || b == c || d < 0 || d > 3 || d == a || d == b || d == c) continue;
      const int perm[4] = {a, b, c, d}; int cost = 0, set[4];
      for (int v = 0; v < 4; ++v) {
        const int pc = pcs[perm[v]], cur = vn_[v + 1]; int bo = 0, bd = 999;
        for (int o = 52; o < 77; ++o) if (o % 12 == pc) { const int dd = o > cur ? o - cur : cur - o; if (dd < bd) { bd = dd; bo = o; } }
        set[v] = bo; cost += bd;
      }
      if (cost < bestCost) { bestCost = cost; for (int v = 0; v < 4; ++v) bestSet[v] = set[v]; }
    }
    for (int v = 0; v < 4; ++v) vn_[v + 1] = bestSet[v];
    for (int v = 0; v < NV; ++v) { vt_[v] = 440.f * pow2((float)(vn_[v] - 69) * (1.f / 12.f)); if (init) vf_[v] = vt_[v]; }
  }
  void start(int note = -1) {
    if (phrase_ == F_A) cur_ = 0;
    else if (phrase_ == F_B) cur_ = 1;
    else if (phrase_ == F_RANDOM) cur_ = (int)(rand01() * 2.f) & 1;
    else if (note >= 0) { const int pc = note % 12; cur_ = (pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10) ? 1 : 0; }
    else { cur_ = alt_; alt_ ^= 1; }
    playing_ = true; pos_ = 0.f;
    dpred_ = 0; didx_ = 0; dn_ = 0; s0_ = s1_ = 0.f;
  }
  inline void decodeNext() {                                     // IMA ADPCM: un campione (dn_), nibble basso prima
    const uint8_t byte = kAdpcm[kPhraseOff[cur_] + (dn_ >> 1)];
    const int c = (dn_ & 1u) ? (byte >> 4) : (byte & 15);
    const int step = kAdpcmStep[didx_]; int vd = step >> 3;
    if (c & 4) vd += step; if (c & 2) vd += step >> 1; if (c & 1) vd += step >> 2;
    dpred_ = (c & 8) ? dpred_ - vd : dpred_ + vd;
    if (dpred_ > 32767) dpred_ = 32767; else if (dpred_ < -32768) dpred_ = -32768;
    didx_ += kAdpcmIdx[c & 7]; if (didx_ < 0) didx_ = 0; else if (didx_ > 88) didx_ = 88;
    s0_ = s1_; s1_ = (float)dpred_; ++dn_;
  }
  int32_t nextWait() {   // EVERY 0 = subito (0,25 s) ... 100% = ~30 s, con pause irregolari
    const float base = 0.25f * pow2(every_ * 6.9f);
    return (int32_t)(base * (0.6f + 0.8f * rand01()) * kSR);
  }
  void control() {
    if (chords_) {
      const int bars = 1 << ((chords_ - 1) & 3);
      ccnt_ += 32;
      if ((float)ccnt_ >= (float)bars * 4.f * 60.f / bpm_ * kSR) { ccnt_ = 0; nextChord(); }
      duck_ += ((playing_ ? 0.5f : 1.f) - duck_) * 0.01f;
      cenv_ += (1.f - cenv_) * 0.004f;                              // piccolo rigonfiamento dopo ogni cambio
      plfo_ += 0.08f * 32.f / kSR; if (plfo_ >= 1.f) plfo_ -= 1.f;  // il filtro del pad respira lentamente
      pc_ = onepole(1100.f + 600.f * sinp(plfo_));
      for (int v = 0; v < NV; ++v) vf_[v] += (vt_[v] - vf_[v]) * 0.06f;   // scivolamento (~20 ms)
    }
    // deriva della sintonia (lenta + fluttuazione) e affievolimento del segnale
    wander1_ += 0.00017f * 32.f; wander2_ += 0.000049f * 32.f; if (wander1_ > 6.2831853f) wander1_ -= 6.2831853f; if (wander2_ > 6.2831853f) wander2_ -= 6.2831853f;
    const float wander = drift_ * drift_ * (0.07f * fsin(wander1_) + 0.04f * fsin(wander2_));
    dial_ += 0.3f * (tune_ - dial_);
    const float d = dial_ + wander - 0.5f, ad = d < 0.f ? -d : d;
    float s = (0.10f - ad) / 0.045f; s = s < 0.f ? 0.f : (s > 1.f ? 1.f : s);          // stazione al centro, con bordi sfumati
    level_ += 0.2f * (s * (0.35f + 0.65f * signal_) - level_);
    const float qsb = 1.f - drift_ * 0.7f * (0.5f + 0.5f * fsin(wander1_ * 1.7f + 1.3f));
    fade_ += 0.05f * (qsb - fade_);
    wob_ = drift_ * 0.006f * fsin(wander1_ * 3.1f);                                       // leggero "tremolio" della voce
    // fischio di battimento: piu' acuto quanto piu' ti allontani dalla stazione, sparisce al centro
    whz_ = 80.f + ad * 9000.f; whl_ = (ad > 0.004f && ad < 0.16f) ? 0.12f * (1.f - s * 0.85f) * (0.6f + 0.4f * (1.f - signal_)) : 0.f;
    // rumori: piu' forti fuori stazione e con SIGNAL basso
    const float open = 1.f - 0.75f * s * signal_;
    hissA_ = noise_ * (0.06f + 0.30f * open);
    crackA_ = noise_ * 0.55f; crackP_ = noise_ * noise_ * 0.0009f + 0.00002f;
    humA_ = noise_ * 0.035f;
  }
  void updateRadio() {
    const float hp = 120.f + 330.f * radio_, lp = 6500.f - 3900.f * radio_;          // banda da ~120 Hz-6,5 kHz a ~450 Hz-2,6 kHz
    hpc_ = onepole(hp); lpc_ = onepole(lp);
    // risonanza "nasale" dell'altoparlantino (~1,3 kHz, fino a +7 dB)
    const float f0 = 1300.f, q = 1.4f, A = pow2(radio_ * 7.f * 0.0830482f), w = 6.2831853f * f0 / kSR;
    const float sn = fsin(w), sh = fsin(0.5f * w), cs = 1.f - 2.f * sh * sh, al = sn / (2.f * q), ia = 1.f / (1.f + al / A);
    pb0_ = (1.f + al * A) * ia; pb1_ = -2.f * cs * ia; pb2_ = (1.f - al * A) * ia; pa1_ = pb1_; pa2_ = (1.f - al / A) * ia;
    drive_ = 1.f + 3.5f * radio_; makeup_ = 1.f / (0.85f + 0.55f * radio_);
  }
  static inline float onepole(float fc) { const float x = 6.2831853f * fc / kSR; return x / (1.f + x); }
  static inline float soft(float x) { return x > 1.5f ? 1.f : (x < -1.5f ? -1.f : x - x * x * x * (4.f / 27.f)); }
  static inline float wrap(float p) { return p - (float)(int32_t)p; }
  static inline float sinp(float ph) { float x = ph < 0.5f ? ph * 4.f - 1.f : 3.f - ph * 4.f; return -x * (1.5f - 0.5f * x * x); }
  static inline float fsin(float x) { const float q0 = x * 0.159154943f; float q = q0 - (float)(int32_t)(q0 >= 0.f ? q0 + 0.5f : q0 - 0.5f); float y = q * 6.28318530718f;
    if (y > 1.5707963f) y = 3.14159265f - y; else if (y < -1.5707963f) y = -3.14159265f - y; const float y2 = y * y;
    return y * (1.f - y2 * (0.16666667f - y2 * (0.0083333333f - y2 * (0.00019841270f - y2 * 0.0000027557319f)))); }
  static inline float pow2(float x) { const bool ng = x < 0.f; if (ng) x = -x; const int32_t ip = (int32_t)x; const float f = x - ip;
    const float r = (float)(1u << (ip > 30 ? 30 : ip)) * (1.f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * 0.00961813f)))); return ng ? 1.f / r : r; }
  inline uint32_t nr() { rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5; return rng_; }
  inline float noise() { return (float)(nr() & 65535u) * (1.f / 32767.5f) - 1.f; }
  inline float crand() { crng_ ^= crng_ << 13; crng_ ^= crng_ >> 17; crng_ ^= crng_ << 5; return (crng_ >> 8) * (1.f / 16777216.f); }   // generatore degli accordi (separato)
  inline float rand01() { return (nr() >> 8) * (1.f / 16777216.f); }

  float tune_, every_, signal_, radio_, noise_, drift_, ghost_; uint8_t key_, phrase_; int cur_, alt_;
  int32_t dpred_ = 0, didx_ = 0; uint32_t dn_ = 0; float s0_ = 0.f, s1_ = 0.f;
  uint32_t crng_ = 0x1C0FFEE5u; uint8_t chords_ = 0; int rootPc_ = 9, deg_ = 0, vn_[NV]; uint32_t ccnt_ = 0; float bpm_ = 120.f, cenv_ = 0.f, duck_ = 1.f, plfo_ = 0.f, pc_ = 0.12f, plp1_ = 0.f, plp2_ = 0.f;
  float vf_[NV], vt_[NV], ph1_[NV], ph2_[NV];
  uint32_t rng_, tick_; float dial_, level_, fade_, pos_, pitch_, wob_ = 0.f; bool playing_; int32_t wait_;
  float hp1_, hp2_, lp1_, lp2_, hpc_, lpc_, pb0_, pb1_, pb2_, pa1_, pa2_, pz1_[2], pz2_[2], drive_, makeup_;
  float wph_, whz_ = 100.f, whl_ = 0.f, humph_, crack_, crackA_ = 0.f, crackP_ = 0.f, burst_, hissLP_, hissA_ = 0.f, humA_ = 0.f, vl_;
  int16_t echo_[EN]; uint32_t ew_, edec_; float elfo_, wander1_, wander2_;
};
