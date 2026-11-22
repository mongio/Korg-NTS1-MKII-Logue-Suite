#include <initializer_list>
#include <cstdio>
#include <cmath>
#include <vector>
#define private public
#include "../../logue-sdk/platform/nts-1_mkii/caos/caos.h"
static std::vector<float> M(Caos::kFloats);
int main(){
  printf("1) Salti tra le ali al secondo al variare di SPEED (CHAOS 75%% e 0)\n");
  for(int ch:{767,0}){ printf("   CHAOS %4d:",ch); for(int sv:{0,205,410,512,615,818,1023}){ Caos c; c.init(M.data()); c.setParam(0,sv); c.setParam(1,ch); float in[128]={0},out[128]; int sw=0; float prev=0; double T=60;
      for(int b=0;b<(int)(T*48000)/64;b++){ c.process(in,out,64); if((c.nx_>0.15f&&prev<-0.15f)||(c.nx_<-0.15f&&prev>0.15f)) sw++; if(fabs(c.nx_)>0.15f) prev=c.nx_; } printf("  %d:%.2f",sv,sw/T); } printf("  (SPEED:salti/s)\n"); }
  printf("2) Stabilita': SPEED massimo, tutti i CHAOS, 2 minuti\n");
  { int bad=0; for(int ch:{0,300,600,1023}){ Caos c; c.init(M.data()); c.setParam(0,1023); c.setParam(1,ch); float in[128]={0},out[128]; for(int b=0;b<120*48000/64;b++){ c.process(in,out,64); if(!std::isfinite(c.x_)||fabs(c.x_)>999) bad++; } } printf("   uscite di scala %d\n",bad); }
  printf("3) DTYPE nuovi e livelli (accordo, impostazioni iniziali)\n");
  for(int t=0;t<5;t++){ Caos c; c.init(M.data()); c.setParam(4,t); float in[128],out[128]; double ph[3]={0,0,0}; const double f[3]={110,164.8,220}; double e=0,ex=0,d=0;
    for(int b=0;b<10*48000/64;b++){ for(int j=0;j<64;j++){ double s=0; for(int k=0;k<3;k++){ ph[k]+=f[k]/48000; if(ph[k]>1) ph[k]-=1; s+=2*ph[k]-1; } in[2*j]=in[2*j+1]=(float)(s*0.12); } c.process(in,out,64); if(b>48000/64) for(int j=0;j<64;j++){ e+=out[2*j]*out[2*j]; ex+=in[2*j]*in[2*j]; d+=(out[2*j]-in[2*j])*(out[2*j]-in[2*j]); } }
    printf("   %-8s livello %+5.1f dB, distanza dal dry %+5.1f dB\n",c.paramStr(4,t),10*log10(e/ex),10*log10(d/ex)); }
  printf("4) Estremi: tutto al massimo, tutti i DTYPE e DEST, segnale forte\n");
  { double pk=0; int nan=0; for(int d=0;d<4;d++) for(int t=0;t<5;t++){ Caos c; c.init(M.data()); c.setParam(2,d); c.setParam(4,t); c.setParam(5,100); c.setParam(7,100); c.setParam(3,100); c.setParam(0,1023); c.setParam(1,1023); c.setParam(8,100); float in[128],out[128]; for(int b=0;b<4*48000/64;b++){ for(int j=0;j<64;j++){ float x=0.9f*sinf((b*64+j)*0.04f); in[2*j]=in[2*j+1]=x; } c.process(in,out,64); for(float y:out){ if(!std::isfinite(y)) nan++; pk=fmax(pk,fabs(y)); } } } printf("   picco %.2f, NaN %d\n",pk,nan); }
}
