#pragma once
/*
 *  File: bobina.h  (BOBINA v0.4: knob A a 4 posizioni 0 CLEAR / 1 STOP / 2 PLAY / 3 REC-DUB, registrazione automatica AUTO1-8)
 *  Looper a nastro: trasporto a zone sul knob A, varispeed -2x..+2x con inerzia,
 *  overdub con decadimento, usura progressiva (filtro, saturazione, cadute), wow & flutter.
 *  Buffer stereo int16 in SDRAM. Crossfade di 10 ms sulla giunzione del loop.
 *  Nessuna dipendenza da libm.
 */
#include <stdint.h>

class Bobina {
 public:
  enum { P_ACTION = 0, P_SPEED, P_MIX, P_SYNC, P_DECAY, P_WEAR, P_WOW, P_TONE, P_SPDMODE, P_INERTIA, P_FADE, P_NUM };
  enum { S_EMPTY = 0, S_REC, S_PLAY, S_DUB, S_STOP, S_ARMED };
  enum { Z_CLEAR = 0, Z_STOP, Z_PLAY, Z_REC };                 // = valore del knob A (0..3)
  enum { SY_OFF = 0, SY_BEAT, SY_BAR, SY_AUTO1, SY_AUTO2, SY_AUTO4, SY_AUTO8 };
  static constexpr float kSR = 48000.f;
  static constexpr uint32_t kXf = 480;            // crossfade giunzione (10 ms)

  void init(int16_t *buf, uint32_t frames) {
    buf_ = buf; cap_ = frames;
    action_ = 0.f; pos_ = 1; auto_target_ = 0; spd_target_ = 1.f; spd_ = 1.f; mix_ = 0.f; sync_ = 0; decay_ = 0.9f; wear_ = 0.f;
    wow_ = 0.15f; tone_ = 1.f; step_ = true; inertia_ = 0.3f; fade_ = 0.f; bpm_ = 120.f;
    state_ = S_EMPTY; len_ = 0; rec_len_ = 0; pre_w_ = 0; for (uint32_t k = 0; k < 2 * kXf; ++k) pre_[k] = 0; ip_ = 0; fp_ = 0.f; last_w_ = 0xFFFFFFFFu;
    gain_ = 0.f; zone_now_ = pos_; zone_applied_ = zone_now_; zone_cnt_ = 0; settle_ = (int32_t)(kSR * 0.5f);
    rng_ = 0x424F4249u; wph_ = fph_ = 0.f; lpL_ = lpR_ = 0.f; wlL_ = wlR_ = 0.f; drop_ = 1.f; drop_t_ = 1.f;
  }

  void setParam(uint8_t id, int32_t v) {
    switch (id) {
      case P_ACTION: pos_ = (uint8_t)(v < 0 ? 0 : (v > 3 ? 3 : v)); break;
      case P_SPEED:  spd_raw_ = (v - 512) * (1.f / 256.f); spd_target_ = shapeSpeed(spd_raw_); break;
      case P_MIX:    mix_ = v * 0.001f; break;
      case P_SYNC:   sync_ = (uint8_t)v; break;
      case P_DECAY:  decay_ = v * 0.01f; break;
      case P_WEAR:   wear_ = v * 0.01f; break;
      case P_WOW:    wow_ = v * 0.01f; break;
      case P_TONE:   tone_ = v * 0.01f; break;
      case P_SPDMODE: step_ = (v != 0); spd_target_ = shapeSpeed(spd_raw_); break;
      case P_INERTIA: inertia_ = v * 0.01f; break;
      case P_FADE:   fade_ = v * 0.01f; break;
      default: break;
    }
  }

  const char *paramStr(uint8_t id, int32_t v) const {
    static const char *syncs[] = {"OFF", "BEAT", "BAR", "AUTO1", "AUTO2", "AUTO4", "AUTO8"};
    static const char *modes[] = {"FREE", "STEP"};
    if (id == P_SYNC && v >= 0 && v < 7) return syncs[v];
    if (id == P_SPDMODE && v >= 0 && v < 2) return modes[v];
    return nullptr;
  }

  void setTempo(float bpm) { if (bpm > 10.f && bpm < 1000.f) bpm_ = bpm; }
  uint8_t state() const { return state_; }
  uint32_t length() const { return len_; }

  void process(const float *in, float *out, uint32_t frames) {
    const float m = (mix_ + 1.f) * 0.5f;
    const float dry_g = m < 0.5f ? 1.f : 2.f * (1.f - m);
    const float wet_g = m > 0.5f ? 1.f : 2.f * m;
    // TONE: passa-basso 800 Hz .. 20 kHz
    const bool tone_on = tone_ < 0.995f;
    float x = 6.2831853f * (800.f * pow2(tone_ * 4.64f)) / kSR; if (x > 3.f) x = 3.f;
    const float tc = x / (1.f + x);
    // inerzia: 0 = immediato, 100% = ~1,5 s per fermare/partire
    const float slew = 1.f / (1.f + inertia_ * inertia_ * 72000.f);
    // usura: un filtro che si chiude un po' a ogni giro
    const float wear_c = 1.f - wear_ * 0.62f;
    const float pass_fade = 1.f - fade_ * 0.25f;   // per giro, distribuito sui campioni

    for (uint32_t i = 0; i < frames; ++i) {
      const float inL = in[2 * i], inR = in[2 * i + 1];
      handleZones();

      if (state_ != S_REC) {                   // pre-roll: gli ultimi 10 ms prima di un eventuale REC
        pre_[2 * pre_w_] = (int16_t)(clip(inL) * 32767.f); pre_[2 * pre_w_ + 1] = (int16_t)(clip(inR) * 32767.f);
        if (++pre_w_ >= kXf) pre_w_ = 0;
      }
      float oL = 0.f, oR = 0.f;
      if (state_ == S_ARMED) {                  // AUTO: la registrazione parte quando cominci a suonare
        const float a = (inL < 0.f ? -inL : inL) + (inR < 0.f ? -inR : inR);
        if (a > 0.03f) startRec();
      }
      if (state_ == S_REC) {
        // registrazione a velocita' 1, da zero
        if (rec_len_ < cap_ - kXf - 2) {
          put(rec_len_, inL, inR);
          ++rec_len_;
          if (auto_target_ > 0 && rec_len_ >= auto_target_) closeLoop();   // AUTO: N battute esatte, poi suona
        } else {
          closeLoop();                          // memoria finita: chiude e suona
          state_ = S_PLAY;
        }
      } else if (len_ > 0) {
        const bool running = (state_ == S_PLAY || state_ == S_DUB);
        const bool tape_stop = inertia_ > 0.05f;
        // STOP con inerzia: il nastro rallenta fino a fermarsi prima di sparire
        const float tgt = running ? spd_target_ : (tape_stop ? 0.f : spd_);
        spd_ += (tgt - spd_) * slew;
        const float sa = spd_ < 0.f ? -spd_ : spd_;
        const float gtarget = running ? 1.f : ((tape_stop && sa > 0.03f) ? 1.f : 0.f);
        gain_ += (gtarget - gain_) * 0.004f;
        const float spd = spd_;

        if (gain_ > 0.0005f || running) {
          // wow & flutter come piccolo spostamento della testina
          wph_ += 0.55f / kSR; if (wph_ >= 1.f) wph_ -= 1.f;
          fph_ += 6.5f / kSR; if (fph_ >= 1.f) fph_ -= 1.f;
          const float off = wow_ * (90.f * sinp(wph_) + 1.2f * sinp(fph_));

          float rL, rR;
          readLoop((float)ip_ + fp_ + off, rL, rR);

          // scrittura sulla testina: overdub, usura, dissolvenza
          const uint32_t wi = ip_;
          if (wi != last_w_ && (state_ == S_DUB || wear_ > 0.f || fade_ > 0.f) && running) {
            last_w_ = wi;
            float aL, aR; get(wi, aL, aR);
            if (wear_ > 0.f) {
              wlL_ += (aL - wlL_) * wear_c; wlR_ += (aR - wlR_) * wear_c;
              aL = soft(wlL_) * (1.f - wear_ * 0.008f); aR = soft(wlR_) * (1.f - wear_ * 0.008f);
              if (rand01() < wear_ * 0.00002f) drop_t_ = 0.3f + 0.5f * rand01();      // caduta di nastro
              drop_ += (drop_t_ - drop_) * 0.002f; drop_t_ += (1.f - drop_t_) * 0.0004f;
              aL *= drop_; aR *= drop_;
            }
            float k = (fade_ > 0.f) ? passFactor(pass_fade) : 1.f;
            if (state_ == S_DUB) { k *= decay_; aL = aL * k + inL; aR = aR * k + inR; }
            else { aL *= k; aR *= k; }
            put(wi, aL, aR);
          }

          // avanzamento con varispeed (anche all'indietro)
          fp_ += spd;
          int32_t d = (int32_t)fp_; if (fp_ < 0.f && (float)d != fp_) --d;
          fp_ -= (float)d;
          int64_t np = (int64_t)ip_ + d;
          const int64_t L = (int64_t)len_;
          while (np >= L) np -= L;
          while (np < 0) np += L;
          ip_ = (uint32_t)np;

          oL = rL * gain_; oR = rR * gain_;
          if (state_ == S_STOP && gain_ < 0.001f) { ip_ = 0; fp_ = 0.f; }
        }
      }

      if (tone_on) { lpL_ += (oL - lpL_) * tc; lpR_ += (oR - lpR_) * tc; oL = lpL_; oR = lpR_; }
      out[2 * i]     = inL * dry_g + clip(oL) * wet_g;
      out[2 * i + 1] = inR * dry_g + clip(oR) * wet_g;
    }
  }

 private:
  // ---------- trasporto: knob A a 4 posizioni ----------
  //   0 = CLEAR (tenuto 1 s), 1 = STOP, 2 = PLAY, 3 = REC / DUB
  //   3: se non c'e' un loop registra (o, con SYNC AUTO, attende il suono); se c'e' sovraincide.
  //   Lasciare il 3 chiude subito la registrazione o l'overdub.
  void handleZones() {
    const uint8_t z = pos_;
    if (settle_ > 0) { --settle_; zone_now_ = z; zone_applied_ = z; return; }   // all'avvio: nessuna azione
    if (z != zone_now_) {
      if (z == Z_REC) { enterRec(); zone_applied_ = Z_REC; }
      else if (zone_now_ == Z_REC) { leaveRec(); zone_applied_ = (len_ > 0) ? Z_PLAY : Z_STOP; }
      zone_now_ = z; zone_cnt_ = 0;
      return;
    }
    if (z == zone_applied_) return;
    const int32_t need = (z == Z_CLEAR) ? (int32_t)(kSR * 1.0f) : (int32_t)(kSR * 0.12f);
    if (++zone_cnt_ < need) return;
    zone_applied_ = z;
    apply(z);
  }
  void enterRec() {
    if (len_ > 0) {                                    // c'e' un loop: overdub
      if (state_ == S_STOP) restartTape();
      state_ = S_DUB; last_w_ = 0xFFFFFFFFu;
    } else if (sync_ >= SY_AUTO1) {
      state_ = S_ARMED;                                // attende il primo suono
    } else startRec();
  }
  void leaveRec() {
    if (state_ == S_REC) closeLoop();
    else if (state_ == S_DUB) state_ = S_PLAY;
    else if (state_ == S_ARMED) state_ = S_EMPTY;
  }
  void startRec() {
    // congela il pre-roll in coda al buffer (zona mai usata dal loop)
    for (uint32_t k = 0; k < kXf; ++k) {
      const uint32_t src = (pre_w_ + k) % kXf, dst = cap_ - kXf + k;
      buf_[2 * dst] = pre_[2 * src]; buf_[2 * dst + 1] = pre_[2 * src + 1];
    }
    state_ = S_REC; rec_len_ = 0; len_ = 0; gain_ = 0.f;
    auto_target_ = 0;
    if (sync_ >= SY_AUTO1) {
      static const float bars[] = {1.f, 2.f, 4.f, 8.f};
      uint32_t t = (uint32_t)(bars[sync_ - SY_AUTO1] * 4.f * (60.f / bpm_) * kSR);
      if (t + kXf + 2 > cap_) t = cap_ - kXf - 2;
      auto_target_ = t;
    }
  }
  void apply(uint8_t z) {
    switch (z) {
      case Z_CLEAR: state_ = S_EMPTY; len_ = 0; gain_ = 0.f; break;
      case Z_STOP:  if (len_ > 0) state_ = S_STOP; break;
      case Z_PLAY:  if (len_ > 0) { if (state_ == S_STOP) restartTape(); state_ = S_PLAY; } break;
      default: break;
    }
  }
  void restartTape() {
    if (gain_ < 0.01f) { ip_ = 0; fp_ = 0.f; }
    if (inertia_ > 0.05f) spd_ = 0.f;                 // il nastro riparte accelerando
  }
  void closeLoop() {
    uint32_t L = rec_len_;
    if (sync_ > 0) {   // quantizza la lunghezza su battute/battiti del tempo
      const float unit = (60.f / bpm_) * kSR * (sync_ == SY_BEAT ? 1.f : 4.f);   // BAR e AUTO: battuta
      uint32_t n = (uint32_t)((float)L / unit + 0.5f); if (n < 1) n = 1;
      uint32_t q = (uint32_t)(n * unit);
      while (q + kXf + 2 > cap_ && n > 1) { --n; q = (uint32_t)(n * unit); }
      if (q + kXf + 2 > cap_) q = cap_ - kXf - 2;
      for (uint32_t k = L; k < q; ++k) put(k, 0.f, 0.f);    // estende con silenzio
      L = q;
    }
    if (L < kXf * 2) { for (uint32_t k = L; k < kXf * 2; ++k) put(k, 0.f, 0.f); L = kXf * 2; }
    // giunzione: la fine del loop sfuma nel pre-roll, cioe' nell'audio subito prima dell'inizio
    for (uint32_t k = 0; k < kXf; ++k) {
      const float w = (float)(k + 1) / (float)kXf;
      float eL, eR, pL, pR; get(L - kXf + k, eL, eR); get(cap_ - kXf + k, pL, pR);
      put(L - kXf + k, eL * (1.f - w) + pL * w, eR * (1.f - w) + pR * w);
    }
    len_ = L; ip_ = 0; fp_ = 0.f; gain_ = 1.f; last_w_ = 0xFFFFFFFFu; state_ = S_PLAY; auto_target_ = 0;
  }

  // lettura interpolata con varispeed
  inline void readLoop(float p, float &l, float &r) const {
    const float Lf = (float)len_;
    while (p >= Lf) p -= Lf;
    while (p < 0.f) p += Lf;
    const uint32_t i0 = (uint32_t)p; const float f = p - (float)i0;
    const uint32_t i1 = (i0 + 1 >= len_) ? 0 : i0 + 1;
    float aL, aR, bL, bR; get(i0, aL, aR); get(i1, bL, bR);
    l = aL + (bL - aL) * f; r = aR + (bR - aR) * f;
  }

  inline void put(uint32_t i, float l, float r) {
    buf_[2 * i] = (int16_t)(clip(l) * 32767.f);
    buf_[2 * i + 1] = (int16_t)(clip(r) * 32767.f);
  }
  inline void get(uint32_t i, float &l, float &r) const {
    l = buf_[2 * i] * (1.f / 32767.f); r = buf_[2 * i + 1] * (1.f / 32767.f);
  }

  // ogni campione viene riscritto una volta per giro: il fattore per giro si applica direttamente
  inline float passFactor(float per_pass) const { return per_pass; }

  float shapeSpeed(float s) const {
    if (s > 2.f) s = 2.f; if (s < -2.f) s = -2.f;
    if (!step_) return s;
    static const float steps[] = {-2.f, -1.f, -0.5f, 0.f, 0.5f, 1.f, 2.f};
    float best = 0.f, bd = 10.f;
    for (int k = 0; k < 7; ++k) { const float d = s - steps[k] < 0.f ? steps[k] - s : s - steps[k]; if (d < bd) { bd = d; best = steps[k]; } }
    return best;
  }

  static inline float pow2(float x) { const int32_t ip = (int32_t)x; const float f = x - ip;
    return (float)(1u << ip) * (1.f + f * (0.6931472f + f * (0.2402265f + f * 0.0555041f))); }
  static inline float sinp(float ph) {   // sin(2*pi*ph), parabola corretta
    float x = ph < 0.5f ? ph * 4.f - 1.f : 3.f - ph * 4.f;   // triangolo -1..1
    x = x * (1.5f - 0.5f * x * x);                             // approssima la sinusoide
    return x;
  }
  static inline float clip(float x) { return x > 1.f ? 1.f : (x < -1.f ? -1.f : x); }
  static inline float soft(float x) { return x - x * x * x * 0.08f; }
  inline uint32_t nextRand() { uint32_t r = rng_; r ^= r << 13; r ^= r >> 17; r ^= r << 5; rng_ = r; return r; }
  inline float rand01() { return (nextRand() >> 8) * (1.f / 16777216.f); }

  int16_t *buf_ = nullptr; uint32_t cap_ = 0;
  uint8_t pos_ = 1; uint32_t auto_target_ = 0;
  float action_, spd_raw_ = 1.f, spd_target_, spd_, mix_, decay_, wear_, wow_, tone_, inertia_, fade_, bpm_;
  uint8_t sync_; bool step_;
  uint8_t state_, zone_now_, zone_applied_;
  int32_t zone_cnt_, settle_;
  uint32_t len_, rec_len_, ip_, last_w_, pre_w_ = 0;
  int16_t pre_[2 * kXf];
  float fp_, gain_;
  uint32_t rng_;
  float wph_, fph_, lpL_, lpR_, wlL_, wlR_, drop_, drop_t_;
};
