#include <initializer_list>
#include <cstdio>
#include <cmath>
#include <vector>
#include "bocca_old.h"
#include "../../logue-sdk/platform/nts-1_mkii/bocca/bocca.h"
int main(){ const char* mn[]={"LFO","RANDOM","ENV","TURING","LOOP","CHAT"};
  for(int m=0;m<6;m++){ BoccaOld a; Bocca b; a.init(); b.init(); for(auto* x : {(void*)0}){} 
    a.setParam(2,m); b.setParam(2,m); a.setParam(1,1023); b.setParam(1,1023); a.setParam(3,40); b.setParam(3,40);
    double e=0,d=0; float in[128],oa[128],ob[128]; float ph=0;
    for(int k=0;k<4*48000/64;k++){ for(int j=0;j<64;j++){ ph+=110.f/48000; if(ph>1)ph-=1; float s=0.4f*(2*ph-1); in[2*j]=in[2*j+1]=s; }
      a.process(in,oa,64); b.process(in,ob,64); for(int j=0;j<128;j++){ e+=oa[j]*oa[j]; d+=(oa[j]-ob[j])*(oa[j]-ob[j]); } }
    printf("   %-6s differenza vecchia/nuova: %5.1f dB sotto il segnale\n",mn[m],10*log10(e/(d+1e-30))); } }
