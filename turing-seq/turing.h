#pragma once
/*
 *  File: turing.h  (TURING SEQ v1.3: scritte del display corrette, compilazione veloce)
 *  Registro a scorrimento "Turing Machine", quantizzatore in cents
 *  (scale occidentali, orientali e microtonali), clock a tempo,
 *  14 forme d'onda. Nessuna dipendenza da libm.
 */
#include <stdint.h>
#include "unit_osc.h"

class TuringOsc {
 public:
  enum { P_LOCK = 0, P_RANGE, P_LENGTH, P_RATE, P_SCALE, P_WAVE, P_TIMBRE, P_TMOD, P_OCT, P_TRIG, P_NUM };
  enum { W_SAW = 0, W_PULSE, W_TRI, W_SINE, W_SUPER, W_SYNC, W_FM, W_BELL, W_FOLD, W_CHIP, W_VOX, W_ORGAN, W_BITS, W_GRIT, W_NUM };
  enum { RATE_NOTE = 6 };
  enum { NUM_SCALES = 28 };
  enum { T_RESET = 1, T_GATE = 2, T_SLIDE = 4 };

  static constexpr float kSR = 48000.f;

  void init() {
    lock_ = 0.75f; range_ = 12.f; len_ = 8; rate_ = 1; scale_ = 10; wave_ = W_SAW;
    timbre_ = 0.3f; tmod_ = 0.f; oct_ = 0; trig_ = 0; slide_now_ = false;
    bpm_ = 120.f; lfo_ = 0.f;
    rng_ = 0x1234ABCDu;
    reg_ = nextRand();
    steps_ = 0; counter_ = 0.f;
    for (int i = 0; i < 3; ++i) ph_[i] = i * 0.31f;
    ph_mod_ = 0.f; ph_slave_ = 0.f; hold_ = 0.f; held_seg_ = -1; crush_cnt_ = 0.f; crush_val_ = 0.f;
    amp_ = 1.f; notes_held_ = 0;
    updateTarget();
    cur_offset_ = target_offset_;
    timbre_cur_ = timbre_;
  }

  // ---- parametri -------------------------------------------------------
  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_LOCK:   lock_ = v * (1.f / 1023.f); break;
      case P_RANGE:  range_ = v * (36.f / 1023.f); updateTarget(); break;
      case P_LENGTH: len_ = (uint8_t)(v + 2); steps_ = 0; break;
      case P_RATE:   rate_ = (uint8_t)v; break;
      case P_SCALE:  scale_ = (uint8_t)v; updateTarget(); break;
      case P_WAVE:   wave_ = (uint8_t)v; break;
      case P_TIMBRE: timbre_ = v * 0.01f; break;
      case P_TMOD:   tmod_ = v * 0.01f; break;
      case P_OCT:    oct_ = (int8_t)(v - 3); break;          // -3..+3, agisce subito sulla sequenza in corsa
      case P_TRIG:   trig_ = (uint8_t)(v & 7); break;
      default: break;
    }
  }

  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *rates[] = {"1-32", "1-16", "1-8", "1-4", "1-2", "1-1", "NOTE"};
    static const char *waves[] = {"SAW", "PULSE", "TRI", "SINE", "SUPER", "SYNC", "FM",
                                  "BELL", "FOLD", "CHIP", "VOX", "ORGAN", "BITS", "GRIT"};
    static const char *lens[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17",
                                 "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32"};
    static const char *octs[] = {"-3", "-2", "-1", "0", "1", "2", "3"};
    static const char *trigs[] = {"FREE", "RESET", "GATE", "GTRS", "SLIDE", "SLRS", "SLGT", "SLGR"};
    static const char *scales[NUM_SCALES] = {
        "CHROM", "MAJOR", "MINOR", "DORIAN", "PHRYGI", "LYDIAN", "MIXOLYD", "HARMMIN", "MELMIN", "NAPMIN",
        "PENTA", "mPENTA", "BLUES", "WHOLE", "FIFTHS", "OCTAVE", "HUNGMIN", "EGYPT",
        "HIRAJO", "IN SEN", "RYUKYU", "PELOG", "SLENDRO", "BHAIRAV", "HIJAZ", "RAST", "BAYATI", "JUST"};
    switch (id) {
      case P_RATE:   if (v >= 0 && v < 7) return rates[v]; break;
      case P_SCALE:  if (v >= 0 && v < NUM_SCALES) return scales[v]; break;
      case P_WAVE:   if (v >= 0 && v < W_NUM) return waves[v]; break;
      case P_LENGTH: if (v >= 0 && v < 31) return lens[v]; break;
      case P_OCT:    if (v >= 0 && v < 7) return octs[v]; break;
      case P_TRIG:   if (v >= 0 && v < 8) return trigs[v]; break;
      default: break;
    }
    return nullptr;
  }

  void setTempo(float bpm) { if (bpm > 10.f && bpm < 1000.f) bpm_ = bpm; }
  void setLfo(float lfo) { lfo_ = lfo; }

  void noteOn() {
    bool restarted = false;
    if (notes_held_ == 0 && (trig_ & T_RESET)) { restart(); restarted = true; }
    if (notes_held_ < 255) ++notes_held_;
    if (rate_ == RATE_NOTE && !restarted) step();
  }
  void noteOff() { if (notes_held_) --notes_held_; }
  void allNotesOff() { notes_held_ = 0; }

  // ---- render ---------------------------------------------------------
  void process(float base_note, float *out, uint32_t frames) {
    static const float div_quarters[] = {0.125f, 0.25f, 0.5f, 1.f, 2.f, 4.f, 0.f};
    const float step_len = (60.f / bpm_) * kSR * div_quarters[rate_ < 7 ? rate_ : 1];
    // SLIDE alla 303: scivola solo sui passi scelti dal registro, in ~1/3 della durata del passo
    const float tau = (rate_ == RATE_NOTE) ? 960.f : step_len * 0.117f;
    const float slide_coef = 1.f / (1.f + tau);
    const bool gate_on = (trig_ & T_GATE) != 0;
    const float oct = 12.f * oct_;

    for (uint32_t i = 0; i < frames; ++i) {
      if (rate_ != RATE_NOTE) {
        counter_ += 1.f;
        if (counter_ >= step_len) { counter_ -= step_len; if (counter_ >= step_len) counter_ = 0.f; step(); }
      }
      cur_offset_ += (target_offset_ - cur_offset_) * (slide_now_ ? slide_coef : 1.f);

      // timbro: base + modulazione dal registro + Shape LFO
      float tt = timbre_ + tmod_ * step_mod_ + lfo_ * 0.5f;
      tt = clip01(tt);
      timbre_cur_ += (tt - timbre_cur_) * 0.003f;

      float n = base_note + oct + cur_offset_;
      if (n < 0.f) n = 0.f;
      if (n > 151.f) n = 151.f;
      const uint8_t ni = (uint8_t)n;
      float w = osc_w0f_for_note(ni, (uint8_t)((n - ni) * 255.f));
      if (w > 0.45f) w = 0.45f;

      const float s = render(w, timbre_cur_);

      const float amp_target = (!gate_on || (reg_ & 1u)) ? 1.f : 0.f;
      amp_ += (amp_target - amp_) * 0.01f;
      out[i] = s * amp_ * 0.85f;
    }
  }

 private:
  // ---------- utilita' DSP ----------
  static inline float clip01(float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }
  static inline float wrap(float x) { float r = x - (float)(int32_t)x; if (r < 0.f) r += 1.f; if (r >= 1.f) r -= 1.f; return r; }
  static inline float sinw(float x) { return osc_sinf(wrap(x)); }
  static inline float blep(float t, float dt) {
    if (t < dt) { const float x = t / dt; return x + x - x * x - 1.f; }
    if (t > 1.f - dt) { const float x = (t - 1.f) / dt; return x * x + x + x + 1.f; }
    return 0.f;
  }
  static inline float saw(float t, float dt) { return 2.f * t - 1.f - blep(t, dt); }
  static inline float pulse(float t, float dt, float pw) {
    float t2 = t + (1.f - pw); if (t2 >= 1.f) t2 -= 1.f;
    return (t < pw ? 1.f : -1.f) + blep(t, dt) - blep(t2, dt);
  }
  static inline float fold(float x) {  // wavefolder a triangolo
    x = x * 0.25f + 0.25f;
    x = wrap(x);
    return x < 0.5f ? 4.f * x - 1.f : 3.f - 4.f * x;
  }
  inline float randf() { return (nextRand() >> 8) * (1.f / 8388608.f) - 1.f; }
  inline uint32_t nextRand() { uint32_t x = rng_; x ^= x << 13; x ^= x >> 17; x ^= x << 5; rng_ = x; return x; }
  inline void adv(float &p, float dt) { p += dt; if (p >= 1.f) p -= 1.f; }

  // ---------- forme d'onda ----------
  float render(float w, float t) {
    float &p = ph_[0];
    adv(p, w);
    switch (wave_) {
      case W_PULSE: {  // PW 50% -> 5%
        const float pw = 0.5f - t * 0.45f;
        const float m = 2.f * pw - 1.f;   // valor medio (DC) da sottrarre
        return (pulse(p, w, pw) - m) / (1.f - m);
      }
      case W_TRI: {    // triangolo -> rampa (asimmetria)
        const float k = 0.5f + t * 0.49f;
        return p < k ? (2.f * p / k - 1.f) : (1.f - 2.f * (p - k) / (1.f - k));
      }
      case W_SINE: {   // sinusoide + saturazione morbida
        const float x = osc_sinf(p) * (1.f + t * 5.f);
        return x / (1.f + (x < 0.f ? -x : x)) * (1.f + t * 0.4f);
      }
      case W_SUPER: {  // 3 saw scordate fino a +/-35 cents
        const float d = t * 35.f * 0.0005776f;
        const float w1 = w * (1.f + d), w2 = w * (1.f - d);
        adv(ph_[1], w1); adv(ph_[2], w2);
        return (saw(p, w) + saw(ph_[1], w1) + saw(ph_[2], w2)) * 0.45f;
      }
      case W_SYNC: {   // hard sync: slave da 1x a 8x
        const float ratio = 1.f + t * 7.f;
        const float ws = w * ratio < 0.45f ? w * ratio : 0.45f;
        ph_slave_ += ws;
        if (p < w) ph_slave_ = p * ratio;  // reset al giro del master
        ph_slave_ = wrap(ph_slave_);
        return (2.f * ph_slave_ - 1.f - blep(ph_slave_, ws)) * (1.f - p * 0.3f);
      }
      case W_FM: {     // FM 2 operatori 1:1, indice 0..5
        return sinw(p + t * 0.8f * osc_sinf(p));
      }
      case W_BELL: {   // FM rapporto 3.5 (campana / metallico)
        adv(ph_mod_, w * 3.5f > 0.45f ? 0.45f : w * 3.5f);
        return sinw(p + t * 1.2f * osc_sinf(ph_mod_));
      }
      case W_FOLD: {   // seno ripiegato
        return fold(osc_sinf(p) * (1.f + t * 7.f));
      }
      case W_CHIP: {   // 8-bit: pulse 12.5/25/50% e triangolo a 4 bit
        const int sel = (int)(t * 3.99f);
        if (sel == 3) {
          const float tri = p < 0.5f ? 4.f * p - 1.f : 3.f - 4.f * p;
          return (float)(int32_t)((tri + 1.f) * 7.5f) * (2.f / 15.f) - 1.f;
        }
        static const float duty[] = {0.125f, 0.25f, 0.5f};
        const float d = duty[sel];
        const float m = 2.f * d - 1.f;
        return ((p < d ? 1.f : -1.f) - m) / (1.f - m) * 0.9f;
      }
      case W_VOX: {    // pulsar/formante: seno finestrato, formante 1x..13x
        const float r = 1.f + t * 12.f;
        const float win = 0.5f - 0.5f * osc_cosf(p);
        return osc_sinf(wrap(p * r)) * win * 1.15f;
      }
      case W_ORGAN: {  // drawbar additivo: armoniche 1,2,3,4,6,8
        const float a2 = 0.6f, a3 = t * 0.7f, a4 = 0.2f + t * 0.4f, a6 = t * t * 0.5f, a8 = t * t * 0.4f;
        float s = osc_sinf(p) + a2 * sinw(2.f * p) + a3 * sinw(3.f * p) + a4 * sinw(4.f * p);
        if (w * 8.f < 0.45f) s += a6 * sinw(6.f * p) + a8 * sinw(8.f * p);
        return s * 0.38f;
      }
      case W_BITS: {   // saw ridotta di bit e di frequenza di campionamento
        const float levels = 2.f + (1.f - t) * (1.f - t) * 62.f;
        crush_cnt_ += 1.f;
        const float hold = 1.f + t * 15.f;
        if (crush_cnt_ >= hold) { crush_cnt_ -= hold; crush_val_ = ((float)(int32_t)(p * levels) + 0.5f) * (2.f / levels) - 1.f; }
        return crush_val_ * 0.8f;
      }
      case W_GRIT: {   // rumore intonato: valori casuali tenuti per segmenti di periodo
        const int segs = 2 + (int)(t * 30.f);
        const int seg = (int)(p * segs);
        if (seg != held_seg_) { held_seg_ = seg; hold_ = randf(); }
        return hold_ * 0.7f;
      }
      default:
        return saw(p, w);
    }
  }

  // ---------- Turing Machine ----------
  void step() {
    float p_flip;
    if (lock_ >= 0.97f) p_flip = 0.f;
    else if (lock_ <= 0.03f) p_flip = 1.f;
    else p_flip = 1.f - lock_;
    uint32_t bit = (reg_ >> (len_ - 1)) & 1u;
    const float r = (nextRand() >> 8) * (1.f / 16777216.f);
    if (r < p_flip) bit ^= 1u;
    reg_ = (reg_ << 1) | bit;
    ++steps_;
    slide_now_ = (trig_ & T_SLIDE) && ((reg_ >> 7) & 1u);   // il registro decide su quali passi scivolare
    updateTarget();
  }

  void restart() {
    const uint8_t L = len_;
    const uint32_t mask = (L >= 32) ? 0xFFFFFFFFu : ((1u << L) - 1u);
    const uint32_t n = steps_ % L;
    uint32_t low = reg_ & mask;
    if (n) low = ((low >> n) | (low << (L - n))) & mask;
    uint32_t r = 0;
    for (uint8_t b = 0; b < 32; b += L) r |= low << b;
    reg_ = r;
    steps_ = 0; counter_ = 0.f;
    updateTarget();
    cur_offset_ = target_offset_;
  }

  // ---------- scale in cents ----------
  struct Scale { uint16_t period; uint8_t et; uint8_t n; int16_t deg[12]; };

  void updateTarget() {
    static const Scale scales[NUM_SCALES] = {
        {1200, 12, 0, {0}},                                        // CHROM
        {1200, 0, 7, {0, 200, 400, 500, 700, 900, 1100}},          // MAJOR
        {1200, 0, 7, {0, 200, 300, 500, 700, 800, 1000}},          // MINOR
        {1200, 0, 7, {0, 200, 300, 500, 700, 900, 1000}},          // DORIAN
        {1200, 0, 7, {0, 100, 300, 500, 700, 800, 1000}},          // PHRYGIAN
        {1200, 0, 7, {0, 200, 400, 600, 700, 900, 1100}},          // LYDIAN
        {1200, 0, 7, {0, 200, 400, 500, 700, 900, 1000}},          // MIXOLYDIAN
        {1200, 0, 7, {0, 200, 300, 500, 700, 800, 1100}},          // MINORE ARMONICA
        {1200, 0, 7, {0, 200, 300, 500, 700, 900, 1100}},          // MINORE MELODICA
        {1200, 0, 7, {0, 100, 300, 500, 700, 800, 1100}},          // MINORE NAPOLETANA
        {1200, 0, 5, {0, 200, 400, 700, 900}},                     // PENTATONICA MAGGIORE
        {1200, 0, 5, {0, 300, 500, 700, 1000}},                    // PENTATONICA MINORE
        {1200, 0, 6, {0, 300, 500, 600, 700, 1000}},               // BLUES
        {1200, 6, 0, {0}},                                         // TONI INTERI
        {1200, 0, 2, {0, 700}},                                    // TONICA + QUINTA
        {1200, 0, 1, {0}},                                         // SOLO TONICA (ottave)
        {1200, 0, 7, {0, 200, 300, 600, 700, 800, 1100}},          // UNGHERESE MINORE
        {1200, 0, 5, {0, 200, 500, 700, 1000}},                    // EGIZIANA (pentatonica sospesa)
        {1200, 0, 5, {0, 200, 300, 700, 800}},                     // HIRAJOSHI (Giappone)
        {1200, 0, 5, {0, 100, 500, 700, 800}},                     // IN SEN / MIYAKO-BUSHI
        {1200, 0, 5, {0, 400, 500, 700, 1100}},                    // RYUKYU (Okinawa)
        {1200, 0, 7, {0, 120, 270, 540, 670, 785, 950}},           // PELOG (gamelan, approx.)
        {1200, 5, 0, {0}},                                         // SLENDRO (5-TET)
        {1200, 0, 7, {0, 100, 400, 500, 700, 800, 1100}},          // RAGA BHAIRAV = doppia armonica ("araba")
        {1200, 0, 7, {0, 100, 400, 500, 700, 800, 1000}},          // MAQAM HIJAZ = frigia dominante
        {1200, 0, 7, {0, 200, 350, 500, 700, 900, 1050}},          // MAQAM RAST (quarti di tono)
        {1200, 0, 7, {0, 150, 300, 500, 700, 800, 1000}},          // MAQAM BAYATI (quarti di tono)
        {1200, 0, 7, {0, 204, 386, 498, 702, 884, 1088}},          // INTONAZIONE NATURALE
    };
    const Scale &sc = scales[scale_ < NUM_SCALES ? scale_ : 0];

    const float dac = (reg_ & 0xFFu) * (1.f / 255.f);
    const float cents = dac * range_ * 100.f;
    const float period = (float)sc.period;
    const int32_t oct = (int32_t)(cents / period);
    const float rem = cents - oct * period;
    float q;
    if (sc.et) {
      const float st = period / sc.et;
      q = (float)(int32_t)(rem / st + 0.5f) * st;
    } else {
      q = 0.f; float best = 1e9f;
      for (uint8_t k = 0; k <= sc.n; ++k) {
        const float d = (k < sc.n) ? (float)sc.deg[k] : period;  // include il grado al periodo successivo
        const float e = d > rem ? d - rem : rem - d;
        if (e < best) { best = e; q = d; }
      }
    }
    target_offset_ = (oct * period + q) * 0.01f;  // in semitoni

    // valore per T.MOD dagli 8 bit alti del registro (0..1)
    step_mod_ = ((reg_ >> 8) & 0xFFu) * (1.f / 255.f);
  }

  // parametri
  float lock_, range_, timbre_, tmod_, bpm_, lfo_;
  int8_t oct_; bool slide_now_;
  uint8_t len_, rate_, scale_, wave_, trig_, notes_held_;
  // stato
  uint32_t rng_, steps_;
  uint32_t reg_;
  int held_seg_;
  float counter_, amp_, cur_offset_, target_offset_, step_mod_, timbre_cur_;
  float ph_[3], ph_mod_, ph_slave_, hold_, crush_cnt_, crush_val_;
};
