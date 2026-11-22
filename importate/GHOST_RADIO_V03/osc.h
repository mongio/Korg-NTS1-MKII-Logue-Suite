#pragma once
#include <stdint.h>
#include <math.h>
#include "voice_samples.h"

class Osc {
public:
 Osc(){reset();}
 void teardown(){} void resume(){} void suspend(){}
 void reset(){ rng=0x7234abcd; tick=0; tune=.4f; reality=.2f; signal=.8f; drift=.2f; ghost=.5f; scan=0; scanDial=0; dial=.4f; noteHz=.01f; shape=0; staticLP=0; burst=0; phase=0; phase2=0; morsePhase=0; voicePos=0; voiceGap=0; voiceIndex=0; voiceActive=false; currentStation=-1; lastStation=-1; level=0; }
 void setPitch(float x){noteHz=x;} void setShapeLfo(float x){shape=x;}
 void setParameter(uint8_t id,int32_t v){float x=clamp(v*.01f,0,1);switch(id){case 0:tune=x;break;case 1:reality=x;break;case 2:signal=x;break;case 3:drift=x;break;case 4:ghost=x;break;case 5:scan=x;break;}}
 const char *getParameterStrValue(uint8_t,int32_t){return nullptr;}
 void noteOn(uint8_t,uint8_t){} void noteOff(uint8_t){} void allNoteOff(){} void setTempo(float){} void tempo4ppqnTick(uint32_t){} void pitchBend(uint16_t){} void channelPressure(uint8_t){} void aftertouch(uint8_t,uint8_t){}
 void process(const float*,float*out,uint32_t frames){
  static const float centers[6]={.09f,.25f,.40f,.56f,.73f,.90f};
  const float tau=6.28318530718f;
  for(uint32_t i=0;i<frames;i++){
   ++tick;
   if((tick&31u)==0){
    // SCAN 0=off, 1..100=slow..fast; full spectrum ~24..4 seconds.
    if(scan>0.005f){scanDial+=32.f/(48000.f*(24.f-20.f*scan));if(scanDial>=1)scanDial-=1;}
    float target=scan>.005f?scanDial:tune;
    // Fast enough for a manual knob turn; drift does not hide tuning.
    dial+=.35f*(target-dial);
    float d=clamp(dial + drift*.006f*shape,0,1);
    int st=-1;float strength=0;
    for(int j=0;j<6;j++){
     float dist=fabsf(d-centers[j]);
     float s=clamp((.073f-dist)/.027f,0,1); // solid station core and fading edge
     if(s>strength){strength=s;st=j;}
    }
    currentStation=st;
    level+=.22f*(strength-level);
    if(st!=lastStation){voicePos=0;voiceGap=0;voiceActive=(st==2||st==4||st==5);voiceIndex=(st==2?0:st==4?1:2);lastStation=st;}
   }
   float n=noise();staticLP+=.17f*(n-staticLP);
   float hiss=.60f*n+.40f*staticLP;
   if((tick&8191u)==0 && noise()>.40f)burst=1;
   burst*=.9993f;
   float sample=0;
   if(voiceActive){
    uint32_t len=voice_len[voiceIndex];
    uint32_t ix=(uint32_t)voicePos;
    if(ix<len-1){
     float frac=voicePos-(float)ix;
     sample=((float)voice_bank[voiceIndex][ix]*(1-frac)+(float)voice_bank[voiceIndex][ix+1]*frac)*(1.f/127.f);
     // Samples were recorded at 8 kHz; vary playback speed for alien voices.
     float rate=(8000.f/48000.f)*(1.f+reality*(voiceIndex==2?.85f:-.30f));
     voicePos+=rate;
    }else{voiceActive=false;voiceGap=0;}
   }else if(currentStation==2||currentStation==4||currentStation==5){
    voiceGap++;if(voiceGap>uint32_t(48000.f*(.20f+.55f*(1.f-ghost)))){voiceActive=true;voicePos=0;voiceGap=0;}
   }
   phase=wrap(phase+(.003f+.018f*reality+.006f*drift*shape));
   phase2=wrap(phase2+(.011f+.015f*reality));
   float whistle=sinf(tau*phase), alien=sinf(tau*phase2)*sinf(tau*(phase+phase2));
   morsePhase=wrap(morsePhase+(.00020f+.00008f*drift));
   int slot=(int)(morsePhase*32.f);
   float morse=((0xA6D39C52u>>(slot&31))&1u)?1.f:0.f;
   float prog=0;
   switch(currentStation){
    case 0: prog=.75f*morse*sinf(tau*phase2)+.08f*hiss;break;
    case 1: prog=.55f*whistle+.35f*alien+.10f*hiss;break;
    case 2: prog=.95f*sample+.07f*hiss;break;
    case 3: prog=.60f*alien+.35f*sinf(tau*(phase+phase2))+.10f*hiss;break;
    case 4: prog=.90f*sample+.20f*whistle+.12f*hiss;break;
    case 5: prog=.95f*sample+.40f*alien+.08f*hiss;break;
    default:prog=0;break;
   }
   float supernatural=(.45f*alien+.20f*whistle+.35f*sample);
   prog=(1.f-reality)*prog+reality*(.50f*prog+.80f*supernatural);
   // Strong separation between empty frequencies and distinct stations.
   float outNoise=(.26f+.22f*(1.f-signal))*(1.f-.90f*level)*hiss;
   float y=level*(.35f+.65f*signal)*prog +outNoise+.10f*burst*n;
   out[i]=clamp(y*.92f,-.95f,.95f);
  }
 }
private:
 static float clamp(float x,float a,float b){return x<a?a:(x>b?b:x);} static float wrap(float x){return x-floorf(x);}
 float noise(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return float(rng&65535u)*(1.f/32767.5f)-1.f;}
 uint32_t rng,tick;float tune,reality,signal,drift,ghost,scan,scanDial,dial,noteHz,shape,staticLP,burst,phase,phase2,morsePhase,voicePos,level;
 uint32_t voiceGap;int voiceIndex,currentStation,lastStation;bool voiceActive;
};
