#pragma once
#include <stdint.h>
#include <math.h>
// DRONE V6: behavioural implementation from the original 2020 user manual.
// Fold mathematics and timing are approximations, NOT recovered original code.
class Osc {
 struct OvertoneVoice {
  float phase,inc,env,hold,noise,f1[3],f2[3],b0[3],a1[3],a2[3],detune1,detune2,phase2,phase3;
  uint32_t age; int wave,stage;bool active;
  OvertoneVoice():phase(0.f),inc(0.f),env(0.f),hold(0.f),noise(0.f),detune1(1.004f),detune2(.996f),phase2(0.f),phase3(0.f),age(0),wave(0),stage(0),active(false){for(int i=0;i<3;i++){f1[i]=0.f;f2[i]=0.f;b0[i]=0.f;a1[i]=0.f;a2[i]=0.f;}}
 };
public:
 Osc(): sr_(48000.f){reset();}
 void reset(){
  for(int i=0;i<4;i++){used_[i]=false;notes_[i]=0;ages_[i]=0;phase_[i]=0.f;slopOffsets_[i]=0.f;droneInc_[i]=0.f;}
  count_=0;serial_=0;repPhase_=0.f;repPitch_=0.f;repTarget_=0.f;repIndex_=0;wave_=0;repVoice_=0;formantMix_=0.f;for(int k=0;k<3;k++){f1_[k]=0.f;f2_[k]=0.f;} for(int i=0;i<4;i++)voices_[i]=OvertoneVoice(); voiceSerial_=0;
  rng_=0x6d2b79f5u; bend_=0.f;hostPitch_=440.f/48000.f;shapeLfo_=0.f;
  slop_=0.18f;algo_=0;amount_=0.55f;freq_=0.35f;mod_=0;shape_=0.f;alt_=0.5f;scale_=0;octave_=6;attack_=0.f;release_=0.4f;envMode_=0;edit_=0;character_=0.f;drift_=0.f;driftPhaseA_=0.f;driftPhaseB_=0.f;sub_=0.f;spectral_=0.f;spectralPhase_=0.f;for(int i=0;i<4;i++)subPhase_[i]=0.f;editMode_=false;shapeSmooth_=shape_;altSmooth_=alt_; turingMode_=0;turingRate_=35;turingSteps_=8;turingMutate_=15;turingDepth_=85;turingRegister_=0xA5B3u;turingPhase_=0.f;turingRateHz_=0.1f*fastExp2(turingRate_*.01f*6.321928f);turingSelected_=0;for(int i=0;i<4;i++)turingGain_[i]=1.f;
 }
 void teardown(){} void resume(){} void suspend(){}
 void setPitch(float v){hostPitch_=v;}
 void setShapeLfo(float v){shapeLfo_=clip(v,-1.f,1.f);}
 void setParameter(uint8_t id,int32_t v){
  switch(id){
   case 0:alt_=clip(v*.01f,0.f,1.f);break; // physical A
   case 1:shape_=clip(v*.01f,0.f,1.f);break; // physical B
   case 2:slop_=clip(v*.01f,0.f,1.f);updateDroneIncs();break;
   case 3:algo_=v<0?0:(v>19?19:v);break;
   case 4:amount_=clip(v*.01f,0.f,1.f);break;
   case 5:freq_=clip(v*.01f,0.f,1.f);break;
   case 6:mod_=v<0?0:(v>3?3:v);break;
   case 7:scale_=v<0?0:(v>10?10:v);break;
   case 8:editMode_=(v!=0);break;
   case 9:
    switch(edit_){
     case 0:wave_=v<0?0:(v>17?17:v);break;
     case 1:octave_=v<0?0:(v>7?7:v);break;
     case 2:attack_=clip(v*.01f,0.f,1.f);break;
     case 3:release_=clip(v*.01f,0.f,1.f);break;
     case 4:envMode_=v<0?0:(v>2?2:v);break;
     case 5:if(v)clearNotes();break;
     case 6:drift_=clip(v*.01f,0.f,1.f);break;
     case 7:sub_=clip(v*.01f,0.f,1.f);break;
     case 8:spectral_=clip(v*.01f,0.f,1.f);break;
     case 9:character_=clip(v*.01f,0.f,1.f);break;
     case 10:turingMode_=v<0?0:(v>3?3:v);break;
     case 11:turingRate_=v<0?0:(v>100?100:v);turingRateHz_=0.1f*fastExp2(turingRate_*.01f*6.321928f);break;
     case 12:turingSteps_=v<2?2:(v>16?16:v);break;
     case 13:turingMutate_=v<0?0:(v>100?100:v);break;
     case 14:turingDepth_=v<0?0:(v>100?100:v);break;
    }break;
  }
 }
 const char* getParameterStrValue(uint8_t id,int32_t v){
  static const char* alg[20]={"A1","B1","C1","D1","E1","A2","B2","C2","D2","E2", "Soft","Hard","Asym","Sine","Tri","Cheby","Phase","Feed","Ring","Digital"};
  static const char* mod[4]={"Freq","Amount","Shape","Algo"};
  static const char* waves[18]={"Triangle","Sine","Saw","Square","Pulse","SuperSaw","FM Bell","Wavetable","NoiseTone","LoFi","Voice A","Voice E","Voice I","Voice O","Voice U","Choir","Talking","Random W"};
  static const char* scales[11]={"Original","Chromatic","Major","Minor","Pent Maj","Pent Min","Dorian","Phrygian","Whole","Dim","Random SH"};
  if(id==3)return alg[v<0?0:(v>19?19:v)];
  if(id==6)return mod[v<0?0:(v>3?3:v)];
  if(id==7)return scales[v<0?0:(v>10?10:v)];
  static const char* edits[15]={"Wave","Octave","Attack","Release","Env Mode","Reset","Drift","Sub","Spectral","Character","T Mode","T Rate","T Steps","T Mutate","T Depth"};
  static const char* octaves[7]={"-2","-1","0","+1","+2","+3","AUTO"};
  static const char* modes[3]={"Gate","Pluck","Swell"};
  if(id==8)return edits[v<0?0:(v>14?14:v)];
  if(id==9){
   const int mapped=edit_==0?(v*17+50)/100:edit_==1?(v*6+50)/100:edit_==4?(v*2+50)/100:v;
   if(edit_==0)return waves[mapped<0?0:(mapped>17?17:mapped)];
   if(edit_==1)return octaves[mapped<0?0:(mapped>6?6:mapped)];
   if(edit_==4)return modes[mapped<0?0:(mapped>2?2:mapped)];
   if(edit_==5)return v>=50?"Clear":"Idle";
   if(edit_==10){static const char* modesT[4]={"Off","Spot","Gate","Overtone"};const int m=(v*3+50)/100;return modesT[m<0?0:(m>3?3:m)];}
  }
  return 0;
 }
 void selectEdit(int e){edit_=e<0?0:(e>14?14:e);}
 int32_t editValue() const {switch(edit_){case 0:return wave_;case 1:return octave_;case 2:return (int32_t)(attack_*100.f+0.5f);case 3:return (int32_t)(release_*100.f+0.5f);case 4:return envMode_;case 6:return (int32_t)(drift_*100.f+0.5f);case 7:return (int32_t)(sub_*100.f+0.5f);case 8:return (int32_t)(spectral_*100.f+0.5f);case 9:return (int32_t)(character_*100.f+0.5f);case 10:return turingMode_;case 11:return turingRate_;case 12:return turingSteps_;case 13:return turingMutate_;case 14:return turingDepth_;default:return 0;}}
 void noteOn(uint8_t n,uint8_t velocity){
  if(!velocity)return; // NOTE OFF never clears a remembered drone note
  int slot=-1;
  // Repeated pitch refreshes its age; never creates a duplicate voice.
  for(int i=0;i<4;i++)if(used_[i]&&notes_[i]==n){slot=i;break;}
  if(slot<0)for(int i=0;i<4;i++)if(!used_[i]){slot=i;break;}
  if(slot<0){slot=0;for(int i=1;i<4;i++)if(ages_[i]<ages_[slot])slot=i;}
  used_[slot]=true;notes_[slot]=n;ages_[slot]=++serial_;
  // Stable pseudo-random detuning for this voice, refreshed only at Note On.
  // Approximately +/- 30 cents at maximum SLOP; zero at minimum.
  slopOffsets_[slot]=((float)(random32()&0xffffu)/32767.5f)-1.f;
  // Keep phase continuous to prevent hard click on voice replacement.
  recount();updateDroneIncs();
 }
 void noteOff(uint8_t){} // Persistent notes, as specified by original manual.
 void allNoteOff(){} // MIDI all-note-off is not DRONE Reset.
 void setTempo(float){} void tempo4ppqnTick(uint32_t){}
 void pitchBend(uint16_t b){bend_=((float)b-8192.f)*(2.f/8192.f);updateDroneIncs();}
 void channelPressure(uint8_t){} void aftertouch(uint8_t,uint8_t){}
 void process(const float*,float*out,uint32_t frames){
  // Block-rate constants: host modulation is block constant.
  const float lfo=shapeLfo_;
  const float fm=clip(freq_+(mod_==0?lfo*.5f:0.f),0.f,1.f);
  const float amt=clip(amount_+(mod_==1?lfo*.5f:0.f),0.f,1.f);
  int alg=algo_;
  if(mod_==3){alg+=(int)floorf(lfo*4.5f+(lfo>=0.f?0.5f:-0.5f));if(alg<0)alg=0;if(alg>19)alg=19;}
  const int family=alg%5;
  const float drive=1.f+amt*(2.f+fm*(3.f+family*2.f));
  const float repHz=0.5f*fastExp2((1.f-altSmooth_)*7.f);
  const float attackStep=1.f/(sr_*(0.002f+attack_*attack_*4.998f));
  const float releaseStep=1.f/(sr_*(0.005f+release_*release_*9.995f));
  for(uint32_t n=0;n<frames;n++){
   // THALOS 1.1: optional, slow analog-style pitch drift. At zero this
   // branch is skipped and the original 8.9T signal path is untouched.
   float driftRatio=1.f;
   if(drift_>0.f){
    driftPhaseA_=wrap(driftPhaseA_+0.071f/sr_);
    driftPhaseB_=wrap(driftPhaseB_+0.113f/sr_);
    const float a=tri(driftPhaseA_*.5f+.25f);
    const float b=tri(driftPhaseB_*.5f+.17f);
    // Maximum approximately +/- 11 cents; linear ratio approximation.
    driftRatio=1.f+drift_*(0.00043f*a+0.00062f*b);
   }
   shapeSmooth_+=(shape_-shapeSmooth_)*0.00042f;
   altSmooth_+=(alt_-altSmooth_)*0.0007f;
   // SPECTRAL: slow autonomous timbre movement, disabled at zero.
   float spectralWave=0.f;
   if(spectral_>0.f){
    spectralPhase_=wrap(spectralPhase_+0.23f/sr_);
    spectralWave=tri(spectralPhase_*.5f+.25f);
   }
   const float mix=clip(shapeSmooth_+(mod_==2?lfo*.5f:0.f)+spectral_*0.70f*spectralWave,0.f,1.f);
   const float dynamicAmt=clip(amt+spectral_*0.65f*spectralWave,0.f,1.f);
   // Turing clock: rotating register with probabilistic feedback mutation.
   // A zero mutation rate produces an exactly repeating pattern of STEPS bits.
   if(turingMode_ && count_){
    turingPhase_+=turingRateHz_/sr_;
    if(turingPhase_>=1.f){
     turingPhase_-=1.f;
     const uint32_t mask=(1u<<turingSteps_)-1u;
     uint32_t feedback=(turingRegister_>>(turingSteps_-1))&1u;
     if((random32()%100u)<(uint32_t)turingMutate_)feedback^=1u;
     turingRegister_=((turingRegister_<<1)|feedback)&mask;
     turingSelected_=(int)(turingRegister_%(uint32_t)count_);
    }
   }
   float main=0.f;
   if(count_){
    int activeOrdinal=0;
    for(int i=0;i<4;i++)if(used_[i]){
     // Envelope remains smooth at transitions; no clicks or note re-triggers.
     float target=1.f;
     if(turingMode_==1 || turingMode_==2){
      const float depth=turingDepth_*.01f;
      const bool selected=(activeOrdinal==turingSelected_);
      const float floorGain=(turingMode_==1)?0.23f:0.f;
      target=selected?1.f:(1.f-depth*(1.f-floorGain));
     }
     ++activeOrdinal;
     turingGain_[i]+=(target-turingGain_[i])*.00035f;
     const float voiceGain=turingGain_[i];
     const float inc=droneInc_[i];
     phase_[i]=wrap(phase_[i]+inc*driftRatio);
     // SUB: phase accumulator at half the fundamental, independent of main oscillator phase.
     if(sub_>0.f)subPhase_[i]=wrap(subPhase_[i]+0.5f*inc*driftRatio);
     const float y=sinf(6.28318530718f*phase_[i]);
     // Algorithms 0..9: preserve the V6.1 DSP verbatim.
     float folded=0.f;
     if(alg<10){
      switch(family){
       case 0:folded=sinf(1.5707963268f*y*drive);break;
       case 1:folded=sinf(3.1415926536f*y*drive*.5f);break;
       case 2:folded=tri(y*drive*.25f+.25f);break;
       case 3:folded=sinf(6.28318530718f*phase_[i]*(1.f+fm*4.f))*amt+y*(1.f-amt);break;
       default:folded=sinf(1.5707963268f*(y+amt*sinf(6.28318530718f*phase_[i]*(2.f+fm*6.f))));break;
      }
     }else{
      const float x=y*drive;
      switch(alg){
       case 10:folded=fastSaturate(x)*.95f;break;                       // Soft
       case 11:folded=tri(x*.25f+.25f);break;                   // Hard
       case 12:folded=tri((x+(x>0.f?fm*x*x:0.f))*.25f+.25f);break; // Asymmetric
       case 13:folded=sinf(1.5707963268f*x);break;             // Sine
       case 14:folded=tri(phase_[i]*(1.f+floorf(fm*7.f)));break;// Triangle harmonics
       case 15:{float z=clip(y,-1.f,1.f);folded=4.f*z*z*z-3.f*z;break;} // Chebyshev T3
       case 16:folded=sinf(6.28318530718f*wrap(phase_[i]+amt*y*fm));break; // Phase distortion
       case 17:{
        // Preserve a little more low-order harmonic weight in Feedback.
        const float fb=x+amt*sinf(6.28318530718f*phase_[i]*3.f);
        folded=0.90f*fastSaturate(fb)+0.10f*y;
        break;
       } // Feedback-like
       case 18:folded=y*sinf(6.28318530718f*phase_[i]*(2.f+floorf(fm*10.f)));break; // Ring
       default:{float z=tri(x*.25f+.25f);float steps=4.f+floorf(fm*28.f);folded=floorf(z*steps+.5f)/steps;break;} // Digital
      }
     }
     // V8.9T: retain a small fundamental component in extended folds.
     // Cheap linear blend: no extra oscillators, filters or transcendentals.
     // Original algorithms 0..9 remain untouched.
     if(alg>=10) folded=0.84f*folded+0.16f*y;
     const float original=clip((1.f-dynamicAmt)*y+dynamicAmt*folded,-1.f,1.f);
     if(sub_>0.f){
      const float subSignal=sinf(6.28318530718f*subPhase_[i]);
      main+=voiceGain*((1.f-sub_)*original+sub_*subSignal);
     }else main+=voiceGain*original;
    }
    main/=count_;
   }
   float repeats=0.f;
   if(count_){
    const float hz=repHz;
    repPhase_+=hz/sr_;
    if(repPhase_>=1.f){
     repPhase_-=floorf(repPhase_);
     // V2 OVERTONE: drone stays at unity; Turing register chooses the
     // overtone pitch source. DEPTH is probability of Turing selection.
     // At DEPTH=0 the V1.4 random overtone selection is preserved.
     int which=(int)(random32()%((uint32_t)count_));
     if(turingMode_==3 && (random32()%100u)<(uint32_t)turingDepth_)
       which=turingSelected_%count_;
     int idx=0;
     for(int i=0;i<4;i++)if(used_[i]){if(which==0){idx=i;break;}--which;}
     repIndex_=idx;
     int selectedWave=(wave_==17)?(int)(random32()%17u):wave_;
     if(wave_==16)selectedWave=10+(int)(random32()%5u);
     const float pitch=noteHz(octaveRepeatNote(notes_[idx],alg)+bend_)/sr_;
     triggerOvertone(pitch,selectedWave,hz);
    }
    // First note: sound immediately, even before the first clock event.
    if(!voiceSerial_)triggerOvertone(noteHz(octaveRepeatNote(notes_[repIndex_],alg)+bend_)/sr_,wave_==17?0:wave_,hz);
    // DUAL OVERTONES: at most two active envelopes, independently rendered.
    for(int j=0;j<2;j++){
     OvertoneVoice &v=voices_[j];
     if(!v.active)continue;
     if(envMode_!=0){
      if(v.stage==1){v.env+=attackStep;if(v.env>=1.f){v.env=1.f;v.stage=2;}}
      if(v.stage==2){v.hold-=1.f/sr_;if(v.hold<=0.f)v.stage=3;}
      if(v.stage==3){v.env-=releaseStep;if(v.env<=0.f){v.active=false;continue;}}
     }else v.env=1.f;
     v.phase=wrap(v.phase+v.inc);v.phase2=wrap(v.phase2+v.inc*v.detune1);v.phase3=wrap(v.phase3+v.inc*v.detune2);
     repeats+=overtoneWave(v.phase,v.inc,v.wave,v)*v.env*.65f;
    }
   }
   const float level=clip(main*(1.f-mix)+repeats*mix,-1.f,1.f);
   // At CHARACTER=0, output is exactly the previous signal path.
   // Character uses a bounded, continuous soft clip without trig functions.
   const float colored=(character_>0.f)?
      ((1.f-character_)*level+character_*fastSaturate(level*(1.f+2.0f*character_))):level;
   out[n]=colored*.75f;
  }
 }
private:
 static float fastSaturate(float x){const float ax=fabsf(x);return clip(x*(27.f+x*x)/(27.f+9.f*x*x+ax*ax*ax*.05f),-1.f,1.f);}
 static float clip(float x,float a,float b){return x<a?a:(x>b?b:x);}
 static float wrap(float x){return x-floorf(x);}
 static float tri(float x){return 1.f-4.f*fabsf(wrap(x)-.5f);}
 float noteHz(float note)const{return 440.f*powf(2.f,(note-69.f)/12.f);}
 // Fast exp2 approximation only for ALT clock, never for note tuning.
 static float fastExp2(float x){const int i=(int)x;const float f=x-i;return (float)(1<<i)*(1.f+f*(.69314718f+f*(.2402265f+f*.0555041f)));}
 void updateDroneIncs(){
  static const float pol[4]={1.f,-1.f,1.f,-1.f};
  for(int i=0;i<4;i++)if(used_[i]){
   const float cents=slop_*(slopOffsets_[i]*30.f+pol[i]*3.f);
   droneInc_[i]=noteHz((float)notes_[i]+bend_+cents*(1.f/100.f))/sr_;
  }
 }
 uint32_t random32(){rng_^=rng_<<13;rng_^=rng_>>17;rng_^=rng_<<5;return rng_;}
 float octaveRepeatNote(uint8_t root,int alg){
  // AUTO exactly retains the V8 selection, including Random S&H.
  const float note=selectRepeatNote(root,alg);
  if(octave_==6)return note;
  // In V8, non-original scales include a built-in +12 semitone offset;
  // remove it before applying the explicit octave selector.
  const float baseOct=(scale_==0)?(alg%10<5?12.f:24.f):(scale_==10?0.f:12.f);
  const float offset=12.f*(float)(octave_-2);
  return clip(note-baseOct+offset,0.f,127.f);
 }
 float selectRepeatNote(uint8_t root,int alg){
  // V8.7 TEST: ORIGINAL and Random S&H unchanged.
  if(scale_==0)return (float)root+(alg%10<5?12.f:24.f);
  if(scale_==10){ // identical to V8.6
   return clip((float)root+(float)((int)(random32()%49u)-24),0.f,127.f);
  }
  static const uint8_t intervals[9][12]={
   {0,1,2,3,4,5,6,7,8,9,10,11},
   {0,2,4,5,7,9,11,12,14,16,17,19},
   {0,2,3,5,7,8,10,12,14,15,17,19},
   {0,2,4,7,9,12,14,16,19,21,24,26},
   {0,3,5,7,10,12,15,17,19,22,24,27},
   {0,2,3,5,7,9,10,12,14,15,17,19},
   {0,1,3,5,7,8,10,12,13,15,17,19},
   {0,2,4,6,8,10,12,14,16,18,20,22},
   {0,2,3,5,6,8,9,11,12,14,15,17}
  };
  // Keep the V8.6 four-note random source and melodic interval generator.
  const int row=scale_-1;
  const int interval=intervals[row][random32()%12u];
  const int candidate=(int)root+12+interval;
  // The oldest *currently active* MIDI note is the global scale tonic.
  // Its slot may change at each fifth distinct MIDI Note On (FIFO).
  int oldest=-1;
  for(int i=0;i<4;i++)if(used_[i]&&(oldest<0||ages_[i]<ages_[oldest]))oldest=i;
  const int tonic=(oldest>=0)?(int)notes_[oldest]:(int)root;
  // Quantize to the nearest pitch class of the selected scale, with
  // deterministic downward tie-breaking. Candidate's register is retained.
  int best=candidate;
  int bestDistance=999;
  for(int n=candidate-12;n<=candidate+12;n++){
   int degree=(n-tonic)%12;
   if(degree<0)degree+=12;
   bool allowed=false;
   for(int k=0;k<12;k++)if((intervals[row][k]%12)==degree){allowed=true;break;}
   if(!allowed)continue;
   const int distance=(n>candidate)?(n-candidate):(candidate-n);
   if(distance<bestDistance){bestDistance=distance;best=n;}
  }
  return clip((float)best,0.f,127.f);
 }
 // PolyBLEP: band-limit the discontinuities of saw/pulse oscillators.
 static float blep(float t,float dt){
  if(dt<=0.f)return 0.f;
  if(t<dt){t/=dt;return t+t-t*t-1.f;}
  if(t>1.f-dt){t=(t-1.f)/dt;return t*t+t+t+1.f;}
  return 0.f;
 }
 float saw(float t,float dt)const{return 2.f*t-1.f-blep(t,dt);}
 float pulse(float t,float dt,float width)const{
  float y=t<width?1.f:-1.f;
  y+=blep(t,dt);y-=blep(wrap(t-width+1.f),dt);return y;
 }
 // Stable biquad band-pass formants, coefficients computed ONLY at note trigger.
 // A normalized band-pass has bounded gain at resonance, unlike the old
 // high-feedback SVF that could ring/overload especially on low notes.
 void setupFormants(OvertoneVoice &v,int vowel){
  static const float hz[5][3]={{800,1150,2900},{400,1700,2600},{300,2200,3000},{450,800,2830},{325,700,2530}};
  static const float q[3]={3.2f,4.2f,5.0f};
  for(int j=0;j<3;j++){
   const float fc=clip(hz[vowel][j],40.f,sr_*.44f);
   const float w=6.28318530718f*fc/sr_;
   const float alpha=sinf(w)/(2.f*q[j]);
   const float inv=1.f/(1.f+alpha);
   v.b0[j]=alpha*inv;
   v.a1[j]=-2.f*cosf(w)*inv;
   v.a2[j]=(1.f-alpha)*inv;
   v.f1[j]=v.f2[j]=0.f;
  }
 }
 float formant(float excitation,int vowel,OvertoneVoice &v){
  static const float gain[5][3]={{.72f,.42f,.23f},{.75f,.45f,.20f},{.72f,.48f,.21f},{.78f,.46f,.20f},{.78f,.42f,.16f}};
  float sum=0.f;
  for(int j=0;j<3;j++){
   // Transposed direct-form-II band-pass, state values bounded by stable poles.
   const float y=v.b0[j]*excitation+v.f1[j];
   const float z=v.f2[j]-v.a1[j]*y;
   v.f2[j]=-v.b0[j]*excitation-v.a2[j]*y;
   v.f1[j]=z;
   sum+=gain[vowel][j]*y;
  }
  return clip(sum*.62f,-.85f,.85f); // conservative formant headroom
 }
 // Bandlimited triangle from two integrated PolyBLEP square edges.
 float triangleBL(float phase,float dt)const{
  // Fourier harmonics limited to Nyquist. Keep low-frequency costs bounded.
  const int nmax=(int)clip(floorf(.48f/dt),1.f,7.f);
  float sum=0.f;
  for(int k=1;k<=nmax;k+=2){
   const float sign=((k&2)==0)?1.f:-1.f;
   sum+=sign*sinf(6.28318530718f*(float)k*phase)/((float)k*(float)k);
  }
  return sum*.810569469f; // 8/pi^2
 }
 float overtoneWave(float phase,float inc,int w,OvertoneVoice &voice){
  const float dt=clip(inc,0.000001f,.49f);
  if(w==0)return triangleBL(phase,dt); // bandlimited triangle, V7 timbre preserved approximately
  if(w==1)return sinf(6.28318530718f*phase);
  if(w==2)return saw(phase,dt);
  if(w==3)return pulse(phase,dt,.5f)*.8f;
  if(w==4)return pulse(phase,dt,.18f)*.7f;
  if(w==5){ // 3 detuned band-limited saws
   return (saw(phase,dt)+saw(voice.phase2,clip(dt*voice.detune1,.000001f,.49f))+saw(voice.phase3,clip(dt*voice.detune2,.000001f,.49f)))*.27f;
  }
  if(w==6){float index=clip((.45f/dt-1.f)*.32f,0.f,2.3f);return .75f*sinf(6.28318530718f*phase+index*sinf(12.56637061436f*phase));}
  if(w==7)return .65f*sinf(6.28318530718f*phase)+(dt*3.f<.49f?.25f*sinf(18.84955592f*phase):0.f)+(dt*5.f<.49f?.10f*sinf(31.4159265f*phase):0.f);
  if(w==8)return .75f*sinf(6.28318530718f*phase)+.25f*voice.noise;
  if(w==9){float q=floorf((saw(phase,dt)+1.f)*7.5f+.5f)/7.5f-1.f;return clip(q,-1.f,1.f);}
  if(w>=10&&w<=14)return formant(saw(phase,dt)*.65f,w-10,voice);
  if(w==15){ // warm chorus A, small amplitude variation, fixed vowel
   float v=formant(saw(phase,dt)*.65f,0,voice);
   return .8f*v+.12f*sinf(6.28318530718f*phase);
  }
  if(w==16)return formant(saw(phase,dt)*.65f,voice.wave-10,voice);
  return tri(phase);
 }
 void triggerOvertone(float inc,int wave,float hz){
  // Reuse inactive voice first; otherwise steal the oldest of two.
  int slot=!voices_[0].active?0:(!voices_[1].active?1:(voices_[0].age<=voices_[1].age?0:1));
  OvertoneVoice &v=voices_[slot];v=OvertoneVoice();v.active=true;v.age=++voiceSerial_;
  v.inc=clip(inc,0.f,.49f);v.wave=wave;if(wave>=10&&wave<=16)setupFormants(v,(wave<=14)?wave-10:0);v.noise=((float)(random32()&65535u)/32767.5f)-1.f;
  v.stage=(envMode_==0)?2:1;
  v.env=(envMode_==0)?1.f:0.f;
  // PLUCK is short; SWELL sustains for a fraction of the ALT clock period.
  v.hold=(envMode_==1)?0.06f:clip(0.65f/hz,0.08f,2.f);
 }
 void recount(){count_=0;for(int i=0;i<4;i++)if(used_[i])++count_;}
 void clearNotes(){turingPhase_=0.f;turingRegister_=0xA5B3u;turingSelected_=0;for(int i=0;i<4;i++)turingGain_[i]=1.f;for(int i=0;i<4;i++)voices_[i]=OvertoneVoice();voiceSerial_=0;for(int i=0;i<4;i++){used_[i]=false;ages_[i]=0;phase_[i]=0.f;}count_=0;serial_=0;repPhase_=0.f;repPitch_=0.f;repTarget_=0.f;repIndex_=0;for(int k=0;k<3;k++){f1_[k]=0.f;f2_[k]=0.f;} for(int i=0;i<4;i++)voices_[i]=OvertoneVoice(); voiceSerial_=0;}
 OvertoneVoice voices_[4];uint32_t voiceSerial_;float attack_,release_;int envMode_,edit_;
 float sr_,hostPitch_,shapeLfo_,slop_,amount_,freq_,shape_,alt_,bend_;
 float character_,drift_,driftPhaseA_,driftPhaseB_,sub_,subPhase_[4],spectral_,spectralPhase_;
 bool editMode_;float shapeSmooth_,altSmooth_;
 int turingMode_,turingRate_,turingSteps_,turingMutate_,turingDepth_,turingSelected_;uint32_t turingRegister_;float turingPhase_,turingRateHz_,turingGain_[4];
 float phase_[4],slopOffsets_[4],droneInc_[4],repPhase_,repPitch_,repTarget_;
 uint8_t notes_[4];uint32_t ages_[4],serial_,rng_;bool used_[4];int count_,repIndex_,algo_,mod_,scale_,wave_,octave_,repVoice_;float formantMix_,f1_[3],f2_[3];
};
