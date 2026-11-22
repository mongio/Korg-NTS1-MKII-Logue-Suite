#pragma once
#include <stdint.h>
#include <math.h>
#include "voice_samples.h"

class Osc {
public:
 Osc(){reset();}
 void teardown(){} void resume(){} void suspend(){}
 void reset(){ rng=0x7234abcd; tick=0; tune=.4f; reality=.2f; signal=.8f; drift=.2f; ghost=.5f; scan=0; scanDial=0; dial=.4f; noteHz=.01f; shape=0; staticLP=0; burst=0; phase=0; phase2=0; morsePhase=0; voicePos=0; voiceGap=0; voiceIndex=0; adpcmPred=0; adpcmIndex=0; adpcmCursor=0; adpcmValue=0; voiceActive=false; echoWrite=0; echoState=0; currentStation=-1; lastStation=-1; level=0; keyMode=1; keyStation=-1; keyPitch=1.f; keyGlitch=0; noteHeld=false; }
 void setPitch(float x){noteHz=x;} void setShapeLfo(float x){shape=x;}
 void setParameter(uint8_t id,int32_t v){float x=clamp(v*.01f,0,1);switch(id){case 0:tune=x;break;case 1:reality=x;break;case 2:signal=x;break;case 3:drift=x;break;case 4:ghost=x;break;case 5:scan=x;break;case 6:keyMode=(int)v;keyStation=-1;noteHeld=false;break;}}
 const char *getParameterStrValue(uint8_t,int32_t){return nullptr;}
 void noteOn(uint8_t n,uint8_t){noteHeld=true;
  if(keyMode==1){keyStation=(int)(n%6);scanDial=centersStatic(keyStation);}
  else if(keyMode==2){keyPitch=powf(2.f,((int)n-60)/12.f);}
  else if(keyMode==3){keyStation=(int)(n%6);keyGlitch=48000/4;voiceIndex=(int)(n%6);startVoice();voiceActive=true;}
 } void noteOff(uint8_t){noteHeld=false;} void allNoteOff(){} void setTempo(float){} void tempo4ppqnTick(uint32_t){} void pitchBend(uint16_t){} void channelPressure(uint8_t){} void aftertouch(uint8_t,uint8_t){}
 void process(const float*,float*out,uint32_t frames){
  static const float centers[6]={.09f,.25f,.40f,.56f,.73f,.90f};
  const float tau=6.28318530718f;
  for(uint32_t i=0;i<frames;i++){
   ++tick;
   if((tick&31u)==0){
    // SCAN 0=off, 1..100=slow..fast; full spectrum ~24..4 seconds.
    if(scan>0.005f){scanDial+=32.f/(48000.f*(24.f-20.f*scan));if(scanDial>=1)scanDial-=1;}
    float target=scan>.005f?scanDial:tune;
    if(keyMode==1 && keyStation>=0 && scan<=.005f)target=clamp(centers[keyStation]+(tune-.5f)*.09f,0,1);
    if(keyMode==3 && keyGlitch>0)target=centers[keyStation];
    // Fast enough for a manual knob turn; drift does not hide tuning.
    dial+=.35f*(target-dial);
    // DRIFT: large, slow detuning with fast flutter and occasional reception loss.
    float wander=drift*drift*(.082f*sinf(tick*.00017f)+.045f*sinf(tick*.000049f)+.014f*sinf(tick*.0013f)+.025f*shape);
    float d=clamp(dial + wander,0,1);
    int st=-1;float strength=0;
    for(int j=0;j<6;j++){
     float dist=fabsf(d-centers[j]);
     float s=clamp((.073f-dist)/.027f,0,1); // solid station core and fading edge
     if(s>strength){strength=s;st=j;}
    }
    currentStation=st;
    level+=.22f*(strength-level);
    if(st!=lastStation){startVoice();voiceActive=(st==2||st==4||st==5);voiceIndex=(st==2? (int)(rng%2u) : st==4? 2+(int)(rng%2u) : 4+(int)(rng%2u));lastStation=st;}
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
     while(adpcmCursor<=ix){
      const uint8_t b=voice_bank[voiceIndex][adpcmCursor>>1];
      const uint8_t nib=(adpcmCursor&1u)?(b>>4):(b&15u);
      decodeNibble(nib);
      ++adpcmCursor;
     }
     sample=(float)adpcmValue*(1.f/32768.f);
     // Samples are stored at 6 kHz; vary playback speed for alien voices.
     // Voice correction: undo the unnatural high pitch of V0.4.3.
     // Longer samples, same ADPCM payload; the original data are unchanged.
     static const float voiceSpeed[6]={1.f,1.f,1.f,1.f,1.f,1.f};
     float rate=(6000.f/48000.f)*voiceSpeed[voiceIndex]*(1.f+reality*(voiceIndex==2?.35f:-.16f));
     voicePos+=rate*(keyMode==2?clamp(keyPitch,.3f,3.f):1.f);
    }else{voiceActive=false;voiceGap=0;}
   }else if(currentStation==2||currentStation==4||currentStation==5){
    voiceGap++;if(voiceGap>uint32_t(48000.f*(.15f+1.65f*(1.f-ghost)))){voiceActive=true;voiceIndex=(int)(rng%6u);startVoice();}
   }
   phase=wrap(phase+(.003f+.018f*reality+.006f*drift*shape)*(keyMode==2?clamp(keyPitch,.3f,3.f):1.f));
   phase2=wrap(phase2+(.011f+.015f*reality)*(keyMode==2?clamp(keyPitch,.3f,3.f):1.f));
   float whistle=sinf(tau*phase), alien=sinf(tau*phase2)*sinf(tau*(phase+phase2));
   morsePhase=wrap(morsePhase+(.00020f+.00008f*drift));
   int slot=(int)(morsePhase*32.f);
   float morse=((0xA6D39C52u>>(slot&31))&1u)?1.f:0.f;
   float prog=0;
   switch(currentStation){
    case 0: prog=.75f*morse*sinf(tau*phase2)+.08f*hiss;break;
    case 1: prog=.55f*whistle+.35f*alien+.10f*hiss;break;
    case 2: prog=1.15f*sample+.035f*hiss;break;
    case 3: prog=.60f*alien+.35f*sinf(tau*(phase+phase2))+.10f*hiss;break;
    case 4: prog=1.10f*sample+.12f*whistle+.06f*hiss;break;
    case 5: prog=1.10f*sample+.24f*alien+.04f*hiss;break;
    default:prog=0;break;
   }
   // GHOST is now a distinct spectral layer: audible even outside voice stations.
   float haunting=(.30f*sinf(tau*(phase2*2.3f+phase))+.18f*alien);
   float ghostGate=.5f+.5f*sinf(tick*.000031f);
   float ghostLayer=ghost*ghost*(.50f*haunting+(.35f+.45f*ghostGate)*sample);
   float supernatural=(.45f*alien+.20f*whistle+.35f*sample);
   prog=(1.f-reality)*prog+reality*(.50f*prog+.80f*supernatural);
   // Strong separation between empty frequencies and distinct stations.
   float outNoise=(.26f+.22f*(1.f-signal))*(1.f-.90f*level)*hiss;
   if(keyGlitch>0){--keyGlitch;prog+=.20f*sinf(tau*(phase*3.f+phase2));}
   // Strongly audible DRIFT: amplitude flutter and loss of station lock.
   float flutter=(1.f-drift*.76f*(.5f+.5f*sinf(tick*(.00027f+.00055f*drift)))) ;
   float y=level*(.35f+.65f*signal)*prog*flutter +outNoise+.10f*burst*n;
   // Delayed, pitch-modulated ghost transmission; intensity 0..100%.
   float delayed=echoBuffer[(echoWrite+2048u-1439u)&2047u];
   echoBuffer[echoWrite]=clamp(sample+delayed*.26f*ghost,-1.f,1.f);
   echoWrite=(echoWrite+1u)&2047u;
   y+=ghostLayer*(.18f+.62f*level)+ghost*ghost*.48f*delayed*(.3f+.7f*ghostGate);
   out[i]=clamp(y*.92f,-.95f,.95f);
  }
 }
private:
 void startVoice(){voicePos=0;voiceGap=0;adpcmPred=0;adpcmIndex=0;adpcmCursor=0;adpcmValue=0;}
 void decodeNibble(uint8_t nib){
  static const uint16_t steps[89]={7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767};
  static const int8_t changes[8]={-1,-1,-1,-1,2,4,6,8};
  const int st=steps[adpcmIndex];
  int diff=st>>3;
  if(nib&1)diff+=st>>2;
  if(nib&2)diff+=st>>1;
  if(nib&4)diff+=st;
  adpcmPred+=(nib&8)?-diff:diff;
  if(adpcmPred>32767)adpcmPred=32767;
  if(adpcmPred< -32768)adpcmPred=-32768;
  adpcmIndex+=changes[nib&7];
  if(adpcmIndex<0)adpcmIndex=0;
  if(adpcmIndex>88)adpcmIndex=88;
  adpcmValue=adpcmPred;
 }
 static float centersStatic(int s){static const float c[6]={.09f,.25f,.40f,.56f,.73f,.90f};return c[s%6];}
 static float clamp(float x,float a,float b){return x<a?a:(x>b?b:x);} static float wrap(float x){return x-floorf(x);}
 float noise(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return float(rng&65535u)*(1.f/32767.5f)-1.f;}
 float echoBuffer[2048] = {}; uint32_t echoWrite; float echoState; uint32_t rng,tick;float tune,reality,signal,drift,ghost,scan,scanDial,dial,noteHz,shape,staticLP,burst,phase,phase2,morsePhase,voicePos,level;
 uint32_t voiceGap,adpcmCursor;int adpcmPred,adpcmIndex,adpcmValue;int voiceIndex,currentStation,lastStation;bool voiceActive;int keyMode,keyStation;float keyPitch;uint32_t keyGlitch;bool noteHeld;
};
