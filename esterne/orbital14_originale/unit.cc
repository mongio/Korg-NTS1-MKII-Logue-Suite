#include "unit_modfx.h"
#include "utils/int_math.h"
#include <math.h>
#include <stdint.h>
static int32_t p[UNIT_MODFX_MAX_PARAM_COUNT];
static float lp[2][3],phase=0.f, pan[4],eq[4], randomPos[4],randomEQ[4];
static float sm[10];
struct Biquad {float b0,b1,b2,a1,a2,z1[2],z2[2];};
static Biquad peak[2];
static float targetPos[4], targetEQ[4], wander[4], peakWander[2];
static float phasePrev=0.f;
static void updatePeak(Biquad &f,float hz,float db,float q){
 const float w=6.28318530718f*hz/48000.f, cs=cosf(w), sn=sinf(w);
 const float A=powf(10.f,db*.025f),alpha=sn/(2.f*q);
 const float inv=1.f/(1.f+alpha/A);
 f.b0=(1.f+alpha*A)*inv; f.b1=(-2.f*cs)*inv; f.b2=(1.f-alpha*A)*inv;
 f.a1=(-2.f*cs)*inv;f.a2=(1.f-alpha/A)*inv;
}
static float runPeak(Biquad &f,int c,float x){
 const float y=f.b0*x+f.z1[c];
 f.z1[c]=f.b1*x-f.a1*y+f.z2[c];f.z2[c]=f.b2*x-f.a2*y;
 return y;
}

static uint32_t rng=0x53A41F21u, samples=0;
static float clipf(float x,float lo,float hi){return x<lo?lo:(x>hi?hi:x);}
static float rnd(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return (float)(rng&65535u)*(2.f/65535.f)-1.f;}
static void clearState(){for(int c=0;c<2;c++)for(int k=0;k<3;k++)lp[c][k]=0.f;phase=0.f;phasePrev=0.f;samples=0;peakWander[0]=peakWander[1]=0.f;for(int k=0;k<4;k++){pan[k]=0.f;eq[k]=1.f;randomPos[k]=0.f;randomEQ[k]=0.f;targetPos[k]=0.f;targetEQ[k]=0.f;wander[k]=0.f;}}
__unit_callback int8_t unit_init(const unit_runtime_desc_t *d){
 if(!d)return k_unit_err_undef;
 if(d->target!=unit_header.target)return k_unit_err_target;
 if(!UNIT_API_IS_COMPAT(d->api))return k_unit_err_api_version;
 if(d->samplerate!=48000)return k_unit_err_samplerate;
 if(d->input_channels!=2||d->output_channels!=2)return k_unit_err_geometry;
 for(unsigned i=0;i<UNIT_MODFX_MAX_PARAM_COUNT;i++)p[i]=unit_header.params[i].init;
 for(int i=0;i<10;i++)sm[i]=(float)p[i]*.001f;
 clearState();for(int b=0;b<2;b++){peak[b]={};updatePeak(peak[b],b?1800.f:450.f,0.f,1.f);}return k_unit_err_none;
}
__unit_callback void unit_teardown(){}
__unit_callback void unit_reset(){clearState();}
__unit_callback void unit_resume(){}
__unit_callback void unit_suspend(){}
__unit_callback void unit_render(const float *in,float *out,uint32_t frames){
 const int mode=p[0];
 for(uint32_t n=0;n<frames;n++){
  for(int i=1;i<10;i++)sm[i]+=((float)p[i]*.001f-sm[i])*.002f;
  float speed=sm[1],width=sm[2],amount=sm[3],motion=sm[4],mix=sm[5],depth=sm[6];
  float sweep=sm[7], resonance=sm[8], eqmix=sm[9];
  if((samples++&31u)==0u){
   // 0.012 - 32 Hz, logarithmic speed, updated at control rate.
   const float hz=.012f*powf(2666.6667f,speed);
   phasePrev=phase;
   phase+=hz*32.f/48000.f;
   if(phase>=1.f){phase-=floorf(phase);
    for(int b=0;b<4;b++){targetPos[b]=rnd();targetEQ[b]=rnd();}
    peakWander[0]=rnd();peakWander[1]=rnd();
   }
   // Smooth random trajectories; never step instantaneously at a phase boundary.
   for(int b=0;b<4;b++){
    randomPos[b]+=.09f*(targetPos[b]-randomPos[b]);
    randomEQ[b]+=.09f*(targetEQ[b]-randomEQ[b]);
   }
   const float t=6.28318530718f*phase;
   // Two strong, animated parametric bell filters.
   // SWEEP 1000: roughly 5 octaves of travel per filter.
   // Q 1000: narrow, highly selective peaks (Q up to 18).
   const float q=.65f+17.35f*resonance*resonance;
   for(int k=0;k<2;k++){
    const float offset=k?2.0943951f:0.f;
    float wave=sinf(t+offset);
    if(mode==5)wave=.65f*randomEQ[k]+.35f*sinf(t+offset);
    else if(mode==3)wave=.5f*randomEQ[k]+.5f*sinf(2.f*t+offset);
    const float center=(k?2400.f:480.f)*powf(2.f,5.f*sweep*wave);
    float envelope=cosf(t+(k?1.6f:0.f));
    if(mode==5)envelope=randomEQ[k];
    if(mode==2)envelope=cosf(t*2.f+offset);
    // Up to +/- 24 dB at PEAK EQ=1000, EQ AMT=1000.
    const float db=24.f*amount*eqmix*envelope;
    updatePeak(peak[k],clipf(center,55.f,12500.f),db,q);
   }
   for(int b=0;b<4;b++){
    const float a=t+b*1.570796327f;
    float pos=0.f,env=0.f;
    if(mode==0){ // DEEP: counter-rotating, slow breathing bands
     pos=sinf(t*.35f+b*1.57f);env=cosf(t*.27f+b*1.5f);
    } else if(mode==1){ // ORBIT: opposite trajectories for adjacent bands
     pos=sinf(t*(b&1?-1.f:1.f)+b*.87f);env=cosf(t+b*1.57f);
    } else if(mode==2){ // GALAXY: cyclic expansion and collapse
     const float expand=.5f+.5f*sinf(t*.7f);
     pos=(b&1?-1.f:1.f)*expand*sinf(t+b*.51f);env=cosf(t*2.f+b*.9f);
    } else if(mode==3){ // QUANTUM: random motion plus accelerated orbit
     pos=.8f*randomPos[b]+.2f*sinf(t*3.f+b);env=.8f*randomEQ[b]+.2f*sinf(t*2.f+b);
    } else if(mode==4){ // EQ WAVE: staggered spectral waves
     pos=sinf(a);env=sinf(a);
    } else { // EQ RAND: smooth stochastic spectral gains
     pos=randomPos[b];env=randomEQ[b];
    }
    const float anchor=b==0?.25f:(b==1?.65f:1.f);
    pan[b]=clipf(pos*width*motion*anchor,-1.f,1.f);
    // Dynamic four-band gain: +/-18 dB; stronger in the EQ modes.
    const float db=(mode>=4?18.f:12.f)*amount*env;
    eq[b]=powf(10.f,db*.05f);
    if(mode<4)eq[b]*=(1.f-.55f*depth*(.5f+.5f*cosf(t*.7f+b)));
   }
  }
  // Complementary spectral decomposition. Each input channel is processed independently.
  float wetL=0.f,wetR=0.f;
  for(int c=0;c<2;c++){
   float x=in[2*n+c];
   // EQ before spectral panning; audible in mono, including with SPREAD=0.
   float filtered=runPeak(peak[0],c,x);
   filtered=runPeak(peak[1],c,filtered);
   x=x*(1.f-eqmix)+filtered*eqmix;
   const float coeff[3]={.0233f,.1053f,.3594f};
   for(int k=0;k<3;k++)lp[c][k]+=coeff[k]*(x-lp[c][k]);
   float band[4]={lp[c][0],lp[c][1]-lp[c][0],lp[c][2]-lp[c][1],x-lp[c][2]};
   for(int b=0;b<4;b++){
    float v=band[b]*eq[b];
    // Linear pan coefficients with unity mono sum. 20% native stereo identity retained.
    float left=.5f*(1.f-pan[b]),right=1.f-left;
    wetL+=v*(.8f*left+(c==0?.2f:0.f));
    wetR+=v*(.8f*right+(c==1?.2f:0.f));
   }
  }
  // Two stereo inputs contribute; normalise stereo->spatial sum to avoid runaway.
  // More audible spectral motion while keeping conservative output headroom.
  wetL*=.78f;wetR*=.78f;
  float l=in[2*n]*(1.f-mix)+wetL*mix;
  float r=in[2*n+1]*(1.f-mix)+wetR*mix;
  // Smooth soft limiter, only near high amplitudes.
  out[2*n]=l/(1.f+.18f*fabsf(l));
  out[2*n+1]=r/(1.f+.18f*fabsf(r));
 }
}
__unit_callback void unit_set_param_value(uint8_t id,int32_t v){if(id>=UNIT_MODFX_MAX_PARAM_COUNT)return;p[id]=clipminmaxi32(unit_header.params[id].min,v,unit_header.params[id].max);}
__unit_callback int32_t unit_get_param_value(uint8_t id){return id<UNIT_MODFX_MAX_PARAM_COUNT?p[id]:0;}
__unit_callback const char *unit_get_param_str_value(uint8_t id,int32_t v){
 static const char *names[]={"DEEP","ORBIT","GALAXY","QUANT","EQ WAVE","EQ RAND"};
 return id==0?names[v<0?0:(v>5?5:v)]:nullptr;
}
__unit_callback void unit_set_tempo(uint32_t){}
__unit_callback void unit_tempo_4ppqn_tick(uint32_t){}
