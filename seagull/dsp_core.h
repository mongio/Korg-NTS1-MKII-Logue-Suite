#pragma once
/*
 *  SEAGULL v0.1 - delay con versi di gabbiano lontani iniettati nelle ripetizioni.
 *  Derivato da VOX INJECT v1.1 (stesso delay decimato 2:1, stessa logica di iniezione a ogni ciclo),
 *  con: scelta del verso o RANDOM, trattamento di lontananza (DISTANCE), posizione stereo per richiamo (SPREAD).
 */
#include <stdint.h>
#include "voice_data.h"
namespace gull {
static constexpr int BUF = 16384;                       // delay (SDRAM)
static constexpr int AP1 = 241, AP2 = 557;              // diffusori della lontananza (SDRAM)
static constexpr uint32_t kSdramShorts = BUF, kSdramFloats = AP1 + AP2;
inline float clip(float x) { return x < -1.f ? -1.f : (x > 1.f ? 1.f : x); }
// Ordine dei parametri: i primi tre come il template KORG per i delay (A, B, DEL+B = MIX nativo).
enum { P_TIME = 0, P_FEEDBACK, P_MIX, P_CALL, P_INJECT, P_CHANCE, P_PITCH, P_DEGRADE, P_DISTANCE, P_SPREAD, P_NUM };
struct Engine {
  int16_t *delay = nullptr; float *ap1 = nullptr, *ap2 = nullptr;
  uint32_t write = 0, sample = 0, rng = 0x5EA6011Du, cycle = 0, a1 = 0, a2 = 0;
  float lo = 0, pos = 0, step = 1, gain = 0, lp1 = 0, lp2 = 0, gl = 1, gr = 1;
  int active = -1; int32_t tail_ = 0;
  int32_t p[P_NUM] = {460, 440, -100, 6, 500, 500, 500, 150, 600, 700};
  float smoothMix = .45f, smoothInject = .5f;
  void setBuffers(int16_t *d, float *f) { delay = d; ap1 = f; ap2 = f + AP1; }
  void reset() {
    if (delay) for (int i = 0; i < BUF; i++) delay[i] = 0;
    if (ap1) for (int i = 0; i < AP1 + AP2; i++) ap1[i] = 0.f;
    write = sample = cycle = a1 = a2 = 0; tail_ = 0; rng = 0x5EA6011Du; lo = pos = gain = lp1 = lp2 = 0; active = -1;
  }
  uint32_t random() { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
  float rand01() { return (random() >> 8) * (1.f / 16777216.f); }
  void trigger() {
    // Probabilita' valutata solo all'inizio di ogni ciclo del delay (come VOX INJECT).
    if ((random() % 1000u) >= (uint32_t)p[P_CHANCE] || p[P_INJECT] == 0) return;
    active = (p[P_CALL] >= kNumVoices) ? (int)(random() % (uint32_t)kNumVoices) : p[P_CALL];   // RANDOM = un verso a caso
    pos = 0.f;
    step = (0.65f + 1.f * (p[P_PITCH] / 1000.f)) * (kVoiceRate / 48000.f);
    gain = 1.f;
    // posizione stereo del richiamo: a caso entro SPREAD
    const float pan = (rand01() * 2.f - 1.f) * (p[P_SPREAD] / 1000.f);
    gl = pan > 0.f ? 1.f - pan : 1.f; gr = pan < 0.f ? 1.f + pan : 1.f;
  }
  float voice() {
    if (active < 0) return 0.f;
    const int idx = (int)pos;
    if (idx >= (int)kVoiceLen[active] - 1) { active = -1; return 0.f; }
    const float fr = pos - idx;
    const int8_t *v = kVoiceData + kVoiceStart[active];
    pos += step;
    return ((float)v[idx] * (1.f - fr) + (float)v[idx + 1] * fr) * (1.f / 128.f) * gain;
  }
  inline float allpass(float *b, uint32_t &i, uint32_t len, float x, float g) {
    const float d = b[i]; const float y = d - g * x; b[i] = x + g * y; if (++i >= len) i = 0; return y;
  }
  void process(float left, float right, float &outL, float &outR) {
    if (!delay) { outL = left; outR = right; return; }
    const float dry = .5f * (left + right);
    const uint32_t delayFrames = (uint32_t)(2400.f + (p[P_TIME] / 1023.f) * 30000.f);   // 50..675 ms
    const uint32_t delayHalf = delayFrames >> 1;
    const uint32_t read = (write + BUF - delayHalf) & (BUF - 1);
    const float tap = (float)delay[read] * (1.f / 32768.f);
    if (++cycle >= delayFrames) { cycle = 0; trigger(); }
    // --- il verso, allontanato: meno acuti (2 poli), meno livello, un po' di diffusione ---
    float voc = voice();
    if (active >= 0) tail_ = AP1 + AP2 + 4800;                    // dopo il verso: lascia esaurire filtro e diffusione
    if (tail_ > 0) {
      --tail_;
      const float dist = p[P_DISTANCE] / 1000.f;
      lp1 += (voc - lp1) * dist_c_; lp2 += (lp1 - lp2) * dist_c_; voc = lp2;
      const float g = 0.6f * dist;
      voc = allpass(ap1, a1, AP1, voc, g); voc = allpass(ap2, a2, AP2, voc, g);
      voc *= 1.f - 0.45f * dist;                                  // insieme al filtro: circa -10 dB al massimo
    }
    voc *= smoothInject * .60f;
    const float feedback = .05f + .87f * (p[P_FEEDBACK] / 1023.f);
    const float degraded = .025f + .44f * (p[P_DEGRADE] / 1000.f);
    lo += (tap - lo) * (1.f - degraded);
    const float wet = clip(lo + voc);                               // nel delay entra mono: i gabbiani tornano nelle ripetizioni
    const float writeSignal = clip(dry + wet * feedback);
    if ((sample & 1u) == 0u) { delay[write] = (int16_t)(writeSignal * 32767.f); write = (write + 1) & (BUF - 1); }
    if ((sample & 31u) == 0u) {
      const float mixT = (p[P_MIX] + 1000) / 2000.f;
      smoothMix += (mixT - smoothMix) * .13f; smoothInject += ((p[P_INJECT] / 1000.f) - smoothInject) * .13f;
      updateDistance();
    }
    // in uscita il richiamo nuovo ha la sua posizione stereo, le ripetizioni restano al centro
    const float wl = clip(lo + voc * gl), wr = clip(lo + voc * gr);
    outL = clip(left * (1.f - smoothMix) + wl * smoothMix);
    outR = clip(right * (1.f - smoothMix) + wr * smoothMix);
    sample++;
  }
  // DISTANCE: taglio del passa basso da ~7 kHz (vicino) a ~2,4 kHz (lontano): i versi dei gabbiani stanno tra 2,5 e 4,5 kHz
  float dist_c_ = 0.6f;
  void updateDistance() {
    const float dist = p[P_DISTANCE] / 1000.f;
    float fc = 7000.f; for (int k = 0; k < 28; ++k) if (k < (int)(dist * 28.f)) fc *= 0.9624f;  // 7 kHz -> ~2,4 kHz: ovattato, non cancellato
    const float x = 6.2831853f * fc / 48000.f; dist_c_ = x / (1.f + x);
  }
};
}
