#include <initializer_list>
#include <cstdio>
#include <cmath>
#include <vector>
#define private public
#include "../../logue-sdk/platform/nts-1_mkii/superorbital/orbital.h"
static std::vector<float> M(Orbital::kFloats);
int main(){ printf("DOPPLER: spostamento d'intonazione massimo per satellite (dalla velocita' di variazione del ritardo)\n");
  for(int sp : {200,400,716,1000}) for(int dep : {0,500,1000}){ Orbital o; o.init(M.data()); o.setParam(2,6); o.setParam(0,sp); o.setParam(5,dep); for(int i=0;i<10;i++) o.sm_[i]=o.p_[i];
    float in[128]={0},out[128]; double up[4]={0},dn[4]={0}; float prev[4]; for(int b=0;b<4;b++) prev[b]=o.dly_[b];
    for(int k=0;k<20*48000/64;k++){ o.process(in,out,64); if(k<48000/64) { for(int b=0;b<4;b++) prev[b]=o.dly_[b]; continue; }
      for(int b=0;b<4;b++){ double r = 1.0 - (o.dly_[b]-prev[b])/64.0; double c=1200*log2(r); up[b]=fmax(up[b],c); dn[b]=fmin(dn[b],c); prev[b]=o.dly_[b]; } }
    printf("   SPEED %4d DEPTH %4d:",sp,dep); for(int b=0;b<4;b++) printf("  banda %d %+4.0f/%+4.0f",b,dn[b],up[b]); printf(" cents\n"); } }
