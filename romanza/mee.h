#pragma once
/*
 *  File: mee.h  (ROMANZA v0.3: solo HOLD per la parafonia; v0.2, motore di MEE-NAH v0.4 adattato: banco a 8 kHz con frammenti tagliati sulle parole)
 *  Oscillatore vocale granulare: frammenti di voce (6 kHz, mu-law) risintetizzati
 *  con TD-PSOLA alla nota suonata, mantenendo le formanti della voce originale.
 *  Nessuna dipendenza da libm.
 */
#include <stdint.h>
#include "unit_osc.h"
#include "mee_bank.h"

class MeeNah {
 public:
  enum { P_SCAN = 0, P_SPEED, P_MODE, P_PLAY, P_PITCH, P_FORMANT, P_JITTER, P_VIBRATO, P_VOICES, P_TAPE, P_NUM };
  enum { V_MONO = 0, V_CHOIR, V_PARA2, V_PARA3, V_PARA4, V_OCT, V_FIFTH, V_MAJ, V_MIN, V_SUS4, V_MAJ7, V_MIN7, V_HOLD2, V_HOLD3, V_HOLD4, V_NUM };
  enum { M_FRAG = 0, M_SEQ, M_RAND, M_BABBLE };
  enum { PL_ONESHOT = 0, PL_LOOP, PL_BOUNCE };
  enum { NSTREAM = 4, NGRAIN = 6, NHELD = 8 };
  static constexpr float kSR = 48000.f;
  static constexpr float kSrcStep = (float)MEE_SR / 48000.f;   // 0.125

  void init() {
    scan_ = 0.f; speed_ = 1.f; mode_ = M_FRAG; play_ = PL_LOOP; pitch_mode_ = 0; formant_ = 1.f;
    jitter_ = 0.1f; vib_ = 0.f; voices_ = V_MONO; tape_ = 0.2f; lfo_ = 0.f;
    nheld_ = 0; last_note_ = 60; clock_ = 0; norm_ = 1.f;
    rng_ = 0x6D696E61u; seq_ = 0;
    vib_ph_ = wow_ph_ = flt_ph_ = 0.f;
    lp1_ = lp2_ = lp3_ = 0.f; hiss_lp_ = 0.f; click_ = 0.f;
    for (int s = 0; s < NSTREAM; ++s) {
      st_[s].frag = 0; st_[s].pos = 0.f; st_[s].dir = 1.f; st_[s].spawn = 0.f; st_[s].ended = false;
      st_[s].last_per = (float)MEE_SR / 200.f; st_[s].babble = 0.f;
      st_[s].note = 0.f; st_[s].amp = s == 0 ? 1.f : 0.f; st_[s].amp_t = st_[s].amp; st_[s].on = (s == 0); st_[s].stamp = 0;
      for (int g = 0; g < NGRAIN; ++g) st_[s].g[g].active = false;
    }
    for (int k = 0; k < NSTREAM; ++k) st_[k].det = 1.f;
    configVoices();
    startAll();
  }

  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_SCAN: scan_ = v * (1.f / 1023.f); break;
      case P_SPEED: {
        const float b = v * (1.f / 1023.f);
        speed_ = b < 0.12f ? 0.f : 0.1f * pow2((b - 0.12f) * (4.3219281f / 0.88f));   // 0.1x .. 2x
      } break;
      case P_MODE: mode_ = (uint8_t)v; break;
      case P_PLAY: play_ = (uint8_t)v; break;
      case P_PITCH: pitch_mode_ = (uint8_t)v; break;
      case P_FORMANT: formant_ = semis((float)(v - 12)); break;
      case P_JITTER: jitter_ = v * 0.01f; break;
      case P_VIBRATO: vib_ = v * 0.01f; break;
      case P_VOICES: {
        // elenco del display (v0.5): MONO, CHOIR, HOLD2, HOLD3, HOLD4, OCTAVE ... MIN7 (le vecchie PARA sono state tolte)
        static const uint8_t map[12] = {V_MONO, V_CHOIR, V_HOLD2, V_HOLD3, V_HOLD4, V_OCT, V_FIFTH, V_MAJ, V_MIN, V_SUS4, V_MAJ7, V_MIN7};
        if (v >= 0 && v < 12 && map[v] != voices_) { voices_ = map[v]; configVoices(); }
      } break;
      case P_TAPE: tape_ = v * 0.01f; break;
      default: break;
    }
  }

  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *modes[] = {"FRAG", "SEQ", "RAND", "BABBLE"};
    static const char *plays[] = {"ONESHOT", "LOOP", "BOUNCE"};
    static const char *pitches[] = {"KEY", "MELODY"};
    static const char *voices[] = {"MONO", "CHOIR", "HOLD2", "HOLD3", "HOLD4", "OCTAVE", "FIFTH", "MAJOR", "MINOR", "SUS4", "MAJ7", "MIN7"};
    static const char *forms[] = {"-12", "-11", "-10", "-9", "-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1", "0",
                                  "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
    switch (id) {
      case P_MODE:    if (v >= 0 && v < 4) return modes[v]; break;
      case P_PLAY:    if (v >= 0 && v < 3) return plays[v]; break;
      case P_PITCH:   if (v >= 0 && v < 2) return pitches[v]; break;
      case P_FORMANT: if (v >= 0 && v < 25) return forms[v]; break;
      case P_VOICES:  if (v >= 0 && v < 12) return voices[v]; break;
      default: break;
    }
    return nullptr;
  }

  void setLfo(float l) { lfo_ = l; }

  // ---------- note ----------
  void noteOn(uint8_t note = 0xFF) {
    if (isPara()) {
      if (note == 0xFF) { for (int k = 0; k < nst_; ++k) if (st_[k].on) startStream(k, true); return; }
      last_note_ = note;
      const bool fresh = (nheld_ == 0);
      addHeld(note);
      // PARA: dopo aver rilasciato tutto, una nota nuova apre una frase nuova.
      // HOLD: le note si accumulano; si sostituisce solo la piu' vecchia quando si supera il limite.
      if (fresh && !isHold()) for (int k = 0; k < NSTREAM; ++k) { st_[k].on = false; st_[k].amp_t = 0.f; }
      int k = isHold() ? findStreamHold(note) : findStream();
      Stream &t = st_[k];
      t.note = (float)note; t.on = true; t.amp_t = 1.f; t.stamp = ++clock_;
      nextFragmentEvent();
      startStream(k, false);
    } else {
      if (note != 0xFF) last_note_ = note;
      nextFragmentEvent();
      startAll();
    }
  }

  void noteOff(uint8_t note) {
    if (note == 0xFF) return;
    removeHeld(note);
    // il firmware torna all'ultima nota ancora tenuta (priorita' all'ultima): ci allineiamo per il pitch bend
    if (nheld_ > 0) last_note_ = held_[nheld_ - 1];
    if (!isPara()) return;
    for (int k = 0; k < nst_; ++k) {
      Stream &t = st_[k];
      if (!t.on || (uint8_t)t.note != note) continue;
      if (nheld_ == 0 || isHold()) { t.on = false; return; }   // ultima nota (o HOLD): la voce resta accesa
      t.on = false; t.amp_t = 0.f;
      // una nota ancora tenuta senza voce (rubata prima) riprende questa voce
      for (int h = nheld_ - 1; h >= 0; --h) {
        if (hasStream(held_[h])) continue;
        t.note = (float)held_[h]; t.on = true; t.amp_t = 1.f; t.stamp = ++clock_;
        startStream(k, false);
        break;
      }
      return;
    }
  }

  void allNotesOff() { nheld_ = 0; for (int k = 0; k < NSTREAM; ++k) { st_[k].on = false; if (isHold()) st_[k].amp_t = 0.f; } }

  __attribute__((optimize("O2")))
  void process(float base_note, float *out, uint32_t frames) {
    float n = base_note; if (n < 0.f) n = 0.f; if (n > 151.f) n = 151.f;
    (void)n;
    const float sc = scanEff();
    // altezza per voce: PARA = nota tenuta + pitch bend; accordi/coro = nota + intervallo
    const bool para = isPara();
    const float bend = base_note - (float)last_note_;
    float fnote[NSTREAM], mratio[NSTREAM], vgain[NSTREAM];
    float active = 0.f;
    for (int k = 0; k < nst_; ++k) {
      float nn = para ? st_[k].note + bend : base_note + ivl_[k];
      if (nn < 0.f) nn = 0.f; if (nn > 151.f) nn = 151.f;
      const uint8_t ni = (uint8_t)nn;
      fnote[k] = osc_w0f_for_note(ni, (uint8_t)((nn - ni) * 255.f)) * kSR;
      mratio[k] = semis(nn - 60.f);                     // MELODY: C4 = altezza originale
      vgain[k] = vg_[k];
      active += st_[k].amp_t * vg_[k] * vg_[k];
    }
    float norm_t = 1.f / sqrtApprox(active < 1.f ? 1.f : active);
    if (voices_ >= V_OCT) norm_t *= 1.3f;              // gli accordi sommano note correlate: compensazione
    // filtro anti-immagine: segue lo spostamento di formante
    float fc = (float)MEE_SR * 0.475f * formant_;     // anti-immagine sotto la Nyquist del banco if (fc > 9000.f) fc = 9000.f;
    const float x = 6.2831853f * fc / kSR, lc = x / (1.f + x);
    const float tape_k = 1.f + tape_ * 1.5f, tape_mk = 0.95f * (1.f + tape_k * 0.5f) / tape_k;   // costanti del blocco

    for (uint32_t i = 0; i < frames; ++i) {
      // modulazioni globali
      vib_ph_ += 5.6f / kSR; if (vib_ph_ >= 1.f) vib_ph_ -= 1.f;
      wow_ph_ += 0.55f / kSR; if (wow_ph_ >= 1.f) wow_ph_ -= 1.f;
      flt_ph_ += 6.8f / kSR; if (flt_ph_ >= 1.f) flt_ph_ -= 1.f;
      const float cents = vib_ * 55.f * osc_sinf(vib_ph_)
                        + tape_ * (14.f * osc_sinf(wow_ph_) + 4.f * osc_sinf(flt_ph_));
      const float mod = 1.f + cents * 0.000577623f;     // ~2^(c/1200) per c piccoli

      float sum = 0.f;
      norm_ += (norm_t - norm_) * 0.002f;
      for (int s = 0; s < nst_; ++s) {
        Stream &t = st_[s];
        t.amp += (t.amp_t - t.amp) * 0.004f;            // ~5 ms
        if (t.amp < 0.0005f && t.amp_t <= 0.f) continue; // voce spenta: non consuma CPU
        advance(t, sc, s);
        t.spawn -= 1.f;
        if (t.spawn <= 0.f && !t.ended) spawnGrain(t, fnote[s], mratio[s], mod);
        sum += renderGrains(t) * t.amp * vgain[s];
      }
      float o = sum * norm_;

      // 3 poli passa-basso (anti-immagine 6 kHz)
      lp1_ += (o - lp1_) * lc; lp2_ += (lp1_ - lp2_) * lc; lp3_ += (lp2_ - lp3_) * lc;
      o = lp3_;

      // TAPE: soffio, crepitii, saturazione
      if (tape_ > 0.f) {
        hiss_lp_ += (randf() - hiss_lp_) * 0.3f;
        o += hiss_lp_ * tape_ * 0.012f;
        if (rand01() < tape_ * 0.00025f) click_ = (randf()) * 0.25f * tape_;
        o += click_; click_ *= 0.82f;
        const float z = o * tape_k;
        o = z / (1.f + (z < 0.f ? -z : z) * 0.5f) * tape_mk;
      }
      o *= 0.75f;
      // limitatore morbido sopra 0.8 (asintoto a 1.0)
      const float a = o < 0.f ? -o : o;
      if (a > 0.8f) { const float e = (a - 0.8f) * 5.f; const float lim = 0.8f + 0.2f * e / (1.f + e); o = o < 0.f ? -lim : lim; }
      out[i] = o;
    }
  }

 private:
  struct Grain { bool active; float rp, rinc, age, len, gain, ws, wc, wsd, wcd; };
  struct Stream {
    uint8_t frag; float pos, dir, spawn, last_per, det, babble; bool ended;
    float note, amp, amp_t; bool on; uint32_t stamp;
    Grain g[NGRAIN];
  };

  // ---------- utilita' ----------
  static inline float pow2(float x) {        // 2^x per x in [-8, 8]
    float neg = 1.f;
    if (x < 0.f) { x = -x; neg = -1.f; }
    const int32_t ip = (int32_t)x; const float f = x - ip;
    const float r = (float)(1u << ip) * (1.f + f * (0.6931472f + f * (0.2402265f + f * 0.0555041f)));
    return neg > 0.f ? r : 1.f / r;
  }
  static inline float semis(float s) { return pow2(s * (1.f / 12.f)); }
  inline uint32_t nextRand() { uint32_t r = rng_; r ^= r << 13; r ^= r >> 17; r ^= r << 5; rng_ = r; return r; }
  inline float rand01() { return (nextRand() >> 8) * (1.f / 16777216.f); }
  inline float randf() { return rand01() * 2.f - 1.f; }

  inline float scanEff() const { float s = scan_ + lfo_ * 0.5f; return s < 0.f ? 0.f : (s > 1.f ? 1.f : s); }
  static uint8_t fragAt(float sc) {
    const uint32_t p = (uint32_t)(sc * (MEE_NSAMP - 1));
    uint8_t k = 0;
    while (k + 1 < MEE_NFRAG && kFrags[k + 1].start <= p) ++k;
    return k;
  }
  static inline float sample(int32_t i) {
    if (i < 0) i = 0; else if (i >= MEE_NSAMP) i = MEE_NSAMP - 1;
    return kMuLaw[kBank[i]] * (1.f / 32767.f);
  }
  static inline float sampleLerp(float p) {
    const int32_t i = (int32_t)p; const float f = p - i;
    const float a = sample(i), b = sample(i + 1);
    return a + (b - a) * f;
  }
  void clampToFrag(Stream &t) {
    const MeeFrag &f = kFrags[t.frag];
    const float a = (float)f.start, b = (float)(f.start + f.len - 1);
    if (t.pos < a) t.pos = a; if (t.pos > b) t.pos = b;
  }

  // ---------- voci ----------
  bool isPara() const { return (voices_ >= V_PARA2 && voices_ <= V_PARA4) || voices_ >= V_HOLD2; }
  bool isHold() const { return voices_ >= V_HOLD2; }
  void configVoices() {
    static const int8_t chords[7][4] = {{0, 12, 0, 0}, {0, 7, 12, 0}, {0, 4, 7, 0}, {0, 3, 7, 0}, {0, 5, 7, 0}, {0, 4, 7, 11}, {0, 3, 7, 10}};
    static const uint8_t csize[7] = {2, 3, 3, 3, 3, 4, 4};
    // (gli accordi occupano V_OCT..V_MIN7; HOLD e PARA sono gestiti a parte)
    for (int k = 0; k < NSTREAM; ++k) { ivl_[k] = 0.f; vg_[k] = 1.f; st_[k].det = 1.f; }
    switch (voices_) {
      case V_MONO: nst_ = 1; break;
      case V_CHOIR: nst_ = 3; st_[1].det = 1.0058f; st_[2].det = 0.9942f; vg_[1] = vg_[2] = 0.75f; break;   // +/- 10 cents
      case V_PARA2: nst_ = 2; break;
      case V_PARA3: nst_ = 3; break;
      case V_PARA4: nst_ = 4; break;
      case V_HOLD2: nst_ = 2; break;
      case V_HOLD3: nst_ = 3; break;
      case V_HOLD4: nst_ = 4; break;
      default: { const int c = (voices_ >= V_OCT && voices_ <= V_MIN7) ? voices_ - V_OCT : 0; nst_ = csize[c]; for (int k = 0; k < nst_; ++k) ivl_[k] = chords[c][k]; } break;
    }
    // stato delle voci coerente con la nuova modalita'
    for (int k = 0; k < NSTREAM; ++k) {
      const bool use = isHold() ? false : (isPara() ? (k == 0) : (k < nst_));   // HOLD: si parte in silenzio
      st_[k].on = use; st_[k].amp_t = use ? 1.f : 0.f; st_[k].note = (float)last_note_;
      if (!use) { st_[k].amp = 0.f; for (int g = 0; g < NGRAIN; ++g) st_[k].g[g].active = false; }
    }
    if (!isPara()) startAll();
  }
  void nextFragmentEvent() {
    if (mode_ == M_SEQ) seq_ = (seq_ + 1) % MEE_NFRAG;
    else if (mode_ == M_RAND) seq_ = nextRand() % MEE_NFRAG;
  }
  void startAll() { for (int k = 0; k < nst_; ++k) startStream(k, true); }
  void startStream(int k, bool keep_frag_choice) {
    (void)keep_frag_choice;
    const float sc = scanEff();
    Stream &t = st_[k];
    if (mode_ == M_SEQ || mode_ == M_RAND) t.frag = seq_; else t.frag = fragAt(sc);
    const MeeFrag &f = kFrags[t.frag];
    if (mode_ == M_FRAG || mode_ == M_BABBLE) t.pos = sc * (MEE_NSAMP - 1);
    else t.pos = f.start + sc * (f.len - 1);
    t.pos += k * 180.f;                         // le voci partono sfalsate (niente effetto phasing)
    clampToFrag(t);
    t.dir = 1.f; t.spawn = 0.f; t.ended = false; t.babble = 0.f;
  }
  // HOLD: stessa nota gia' accesa -> riparte quella; poi una voce mai usata/silenziosa; poi la piu' vecchia
  int findStreamHold(uint8_t note) {
    for (int k = 0; k < nst_; ++k) if (st_[k].amp_t > 0.f && (uint8_t)st_[k].note == note) return k;
    for (int k = 0; k < nst_; ++k) if (st_[k].amp_t <= 0.f && st_[k].amp < 0.01f) return k;
    int old = 0; for (int k = 1; k < nst_; ++k) if (st_[k].stamp < st_[old].stamp) old = k;
    return old;
  }
  int findStream() {
    for (int k = 0; k < nst_; ++k) if (!st_[k].on && st_[k].amp < 0.01f) return k;   // libera e silenziosa
    for (int k = 0; k < nst_; ++k) if (!st_[k].on) return k;                         // libera in rilascio
    int old = 0; for (int k = 1; k < nst_; ++k) if (st_[k].stamp < st_[old].stamp) old = k;
    return old;                                                                       // ruba la piu' vecchia
  }
  bool hasStream(uint8_t note) const { for (int k = 0; k < nst_; ++k) if (st_[k].on && (uint8_t)st_[k].note == note) return true; return false; }
  void addHeld(uint8_t note) {
    removeHeld(note);
    if (nheld_ >= NHELD) { for (int h = 1; h < NHELD; ++h) held_[h - 1] = held_[h]; --nheld_; }
    held_[nheld_++] = note;
  }
  void removeHeld(uint8_t note) {
    for (int h = 0; h < nheld_; ++h) if (held_[h] == note) { for (int j = h + 1; j < nheld_; ++j) held_[j - 1] = held_[j]; --nheld_; return; }
  }

  // ---------- testina di lettura ----------
  __attribute__((optimize("O2")))
  void advance(Stream &t, float sc, int s) {
    // MODE FRAG: SCAN sceglie il frammento; con SPEED = 0 la testina segue SCAN dal vivo
    if (mode_ == M_FRAG) {
      const uint8_t f = fragAt(sc);
      if (speed_ <= 0.f) {
        t.frag = f;
        const float target = sc * (MEE_NSAMP - 1) + s * 180.f;
        t.pos += (target - t.pos) * 0.002f;
        clampToFrag(t);
        return;
      }
    }
    if (mode_ == M_BABBLE) {
      t.babble -= 1.f;
      if (t.babble <= 0.f) {   // salto: JITTER = quanto lontano da SCAN
        t.babble = kSR * (0.06f + 0.2f * rand01()) / (speed_ > 0.2f ? speed_ : 0.2f);
        float p = sc + randf() * jitter_ * 0.5f;
        p = p < 0.f ? 0.f : (p > 1.f ? 1.f : p);
        t.frag = fragAt(p);
        t.pos = p * (MEE_NSAMP - 1);
        t.ended = false;
        clampToFrag(t);
      }
    }
    if (speed_ <= 0.f || t.ended) return;
    t.pos += t.dir * speed_ * kSrcStep;
    const MeeFrag &f = kFrags[t.frag];
    const float a = (float)f.start, b = (float)(f.start + f.len - 1);
    if (t.pos >= b || t.pos <= a) {
      const uint8_t pm = (mode_ == M_BABBLE) ? PL_LOOP : play_;
      if (pm == PL_ONESHOT) { t.ended = true; t.pos = t.pos >= b ? b : a; }
      else if (pm == PL_LOOP) { t.pos = t.dir > 0.f ? a : b; }
      else { t.dir = -t.dir; t.pos = t.pos >= b ? b : a; }
    }
  }

  // ---------- PSOLA: un grano = 2 periodi della voce, centrato su un picco ----------
  __attribute__((optimize("O2")))
  void spawnGrain(Stream &t, float f_note, float mel_ratio, float mod) {
    const int32_t frame = (int32_t)t.pos / MEE_HOP;
    const uint8_t pq = kPeriod[frame < (int32_t)sizeof(kPeriod) ? frame : (int32_t)sizeof(kPeriod) - 1];
    const bool voiced = pq != 0;
    float per = voiced ? pq * (1.f / (float)MEE_PSCALE) : t.last_per;
    if (voiced) t.last_per = per;
    const float f_src = (float)MEE_SR / per;

    float f_t = (pitch_mode_ == 1) ? f_src * mel_ratio : f_note;
    f_t *= mod * t.det * (1.f + randf() * jitter_ * 0.02f);
    if (f_t < 20.f) f_t = 20.f;
    float T_out = kSR / f_t;
    // consonanti (non intonate): grani con la spaziatura naturale della sorgente, indipendente dalla nota,
    // cosi' fruscii e attacchi delle parole restano come nell'originale (solo le vocali vengono ritrasposte)
    if (!voiced) { T_out = per / kSrcStep * (0.85f + 0.3f * rand01()); }

    // centro del grano: picco locale vicino alla testina (+ jitter)
    float c = t.pos + randf() * jitter_ * 240.f;
    const int32_t ci = (int32_t)c, half = (int32_t)(per * 0.5f) + 1;
    int32_t best = ci; float bv = -2.f;
    for (int32_t k = ci - half; k <= ci + half; ++k) { const float v = sample(k); if (v > bv) { bv = v; best = k; } }

    const float rinc = kSrcStep * formant_;
    const float glen = 2.f * per / rinc;
    // compensazione di sovrapposizione: attenua solo quando i grani si accavallano (note alte)
    float gain = T_out * rinc / per;
    gain = gain >= 1.f ? 1.f : (gain < 0.09f ? 0.3f : sqrtApprox(gain));

    // slot libero (altrimenti sostituisce il grano piu' vecchio)
    int slot = -1; float oldest = -1.f;
    for (int g = 0; g < NGRAIN; ++g) {
      if (!t.g[g].active) { slot = g; break; }
      if (t.g[g].age > oldest) { oldest = t.g[g].age; slot = g; }
    }
    Grain &gr = t.g[slot];
    gr.active = true; gr.rp = (float)best - per; gr.rinc = rinc; gr.age = 0.f; gr.len = glen; gr.gain = gain;
    { const float th = 3.14159265f / glen, t2 = th * th;                 // passo angolare della finestra sin(pi*ph)
      gr.wsd = th * (1.f - t2 * (1.f / 6.f) * (1.f - t2 * 0.05f)); gr.wcd = 1.f - t2 * 0.5f * (1.f - t2 * (1.f / 12.f));
      gr.ws = 0.f; gr.wc = 1.f; }
    t.spawn += T_out;
    if (t.spawn < 1.f) t.spawn = 1.f;
  }

  __attribute__((optimize("O2")))
  float renderGrains(Stream &t) {
    float acc = 0.f;
    for (int g = 0; g < NGRAIN; ++g) {
      Grain &gr = t.g[g];
      if (!gr.active) continue;
      const float sn = gr.ws;                          // sin(pi*ph) per rotazione, senza divisioni
      { const float ns = gr.ws * gr.wcd + gr.wc * gr.wsd; gr.wc = gr.wc * gr.wcd - gr.ws * gr.wsd; gr.ws = ns; }
      acc += sampleLerp(gr.rp) * sn * sn * gr.gain;
      gr.rp += gr.rinc;
      gr.age += 1.f;
      if (gr.age >= gr.len) gr.active = false;
    }
    return acc;
  }

  static inline float sqrtApprox(float x) {
    float y = x > 1.f ? x * 0.5f : 1.f;
    for (int k = 0; k < 4; ++k) y = 0.5f * (y + x / y);
    return y;
  }

  float scan_, speed_, formant_, jitter_, vib_, tape_, lfo_;
  uint8_t voices_, nst_ = 1, held_[NHELD], nheld_, last_note_;
  float ivl_[NSTREAM], vg_[NSTREAM], norm_;
  uint32_t clock_;
  uint8_t mode_, play_, pitch_mode_, seq_;
  uint32_t rng_;
  float vib_ph_, wow_ph_, flt_ph_, lp1_, lp2_, lp3_, hiss_lp_, click_;
  Stream st_[NSTREAM];
};
