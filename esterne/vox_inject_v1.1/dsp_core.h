#pragma once
#include <stdint.h>
#include "voice_data.h"
namespace vox {
static constexpr int BUF=16384;                 // buffer del delay: allocato in SDRAM (32 KB), non nella RAM interna
inline float clip(float x){return x < -1.f ? -1.f : (x > 1.f ? 1.f : x);}
inline float absf(float x){return x<0.f?-x:x;}
// Ordine dei parametri: i primi tre seguono il template KORG per i delay
// (0 = knob A, 1 = knob B, 2 = knob B tenendo premuto DEL = MIX nativo).
enum { P_TIME=0, P_FEEDBACK, P_MIX, P_VOICE, P_INJECT, P_CHANCE, P_PITCH, P_DEGRADE, P_NUM };
struct Engine {
 int16_t *delay=nullptr;
 uint32_t write=0, sample=0, rng=0xA93B7D41u, cycle=0;
 float prev=0, lo=0, vocalPos=0, vocalStep=1, voiceGain=0;
 int active=-1;
 int32_t p[P_NUM]={460,440,-100,0,500,500,500,150};
 float smoothMix=.45f, smoothInject=.5f;
 void setBuffer(int16_t *b){delay=b;}
 void reset(){ if(delay) for(int i=0;i<BUF;i++)delay[i]=0; write=sample=cycle=0; rng=0xA93B7D41u; prev=lo=vocalPos=voiceGain=0; active=-1; }
 uint32_t random(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
 void trigger(){
  // Probabilita' valutata solo all'inizio di ogni ciclo del delay.
  if((random()%1000u)>=(uint32_t)p[P_CHANCE] || p[P_INJECT]==0)return;
  active=p[P_VOICE] < kNumVoices ? p[P_VOICE] : kNumVoices-1; vocalPos=0.f;
  // Velocita' di lettura 0.65x..1.65x (sorgente 8 kHz, uscita 48 kHz).
  vocalStep=(0.65f+1.f*(p[P_PITCH]/1000.f))*(8000.f/48000.f);
  voiceGain=1.f;
 }
 float voice(){
  if(active<0)return 0.f;
  const int idx=(int)vocalPos;
  if(idx>=(int)kVoiceLen[active]-1){active=-1;return 0.f;}
  const float frac=vocalPos-idx;
  const int8_t *v=kVoiceData+kVoiceStart[active];
  const float s=((float)v[idx]*(1.f-frac)+(float)v[idx+1]*frac)*(1.f/128.f);
  vocalPos+=vocalStep;
  return s*voiceGain;
 }
 void process(float left,float right,float &outL,float &outR){
  if(!delay){outL=left;outR=right;return;}
  const float dry=.5f*(left+right);
  // Delay decimato 2:1: max ~682 ms, buffer mono int16.
  const uint32_t delayFrames=(uint32_t)(2400.f+(p[P_TIME]/1023.f)*30000.f); // 50..675 ms @48k
  const uint32_t delayHalf=delayFrames>>1;
  const uint32_t read=(write+BUF-delayHalf)&(BUF-1);
  const float tap=(float)delay[read]*(1.f/32768.f);
  if(++cycle>=delayFrames){cycle=0;trigger();}      // stesso effetto di (sample % delayFrames)==0, senza divisione per campione
  const float voc=voice()*smoothInject*.60f;
  const float feedback=.05f+.87f*(p[P_FEEDBACK]/1023.f);
  const float degraded=.025f+.44f*(p[P_DEGRADE]/1000.f);
  lo+=(tap-lo)*(1.f-degraded);
  const float wet=clip(lo+voc);
  const float writeSignal=clip(dry+wet*feedback);
  if((sample&1u)==0u){delay[write]=(int16_t)(writeSignal*32767.f);write=(write+1)&(BUF-1);}
  if((sample&31u)==0u){const float mixT=(p[P_MIX]+1000)/2000.f; smoothMix+=(mixT-smoothMix)*.13f; smoothInject+=((p[P_INJECT]/1000.f)-smoothInject)*.13f;}
  outL=clip(left*(1.f-smoothMix)+wet*smoothMix);
  outR=clip(right*(1.f-smoothMix)+wet*smoothMix);
  sample++;
 }
};
}
