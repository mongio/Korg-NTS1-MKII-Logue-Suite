#include <initializer_list>
#include <cstdio>
#include <cmath>
#include <vector>
#include <chrono>
#include "../../logue-sdk/platform/nts-1_mkii/lola-d/dsp_core.h"
static int16_t B[lola::BUF]; static float F[lola::AP1+lola::AP2];
static lola::Engine mk(){ lola::Engine e; e.setBuffers(B,F); e.reset(); e.updateDistance(); return e; }
static double cent(const std::vector<float>&x){ int N=x.size(); double num=0,den=0; for(int k=1;k<N/2;k+=8){ double re=0,im=0; for(int i=0;i<N;i++){ double w=0.5-0.5*cos(2*M_PI*i/N); re+=x[i]*w*cos(2*M_PI*k*i/N); im-=x[i]*w*sin(2*M_PI*k*i/N);} double m=sqrt(re*re+im*im); num+=m*k*48000.0/N; den+=m; } return num/den; }
int main(){ using namespace lola;
  printf("1) Tempo dell'eco: atteso 2400 + TIME*30000/1023 campioni\n");
  for(int tm:{0,512,1023}){ Engine e=mk(); e.p[P_TIME]=tm; e.p[P_INJECT]=0; e.p[P_MIX]=1000; e.p[P_DEGRADE]=0; float L,R; int first=-1; for(int n=0;n<40000;n++){ e.process(n==0?0.8f:0,n==0?0.8f:0,L,R); if(n>100&&first<0&&fabs(L)>0.05f) first=n; } printf("   TIME %4d: atteso %5d, misurato %5d\n",tm,(int)(2400+tm/1023.f*30000),first); }
  printf("2) Ogni verso (CHANCE 1000, ingresso muto, DISTANCE 0): livello\n   ");
  const char* nm[]={"BAU1","BAU2","BAU3","BAU4","BAU5","BAU6","BAU7","BAU8"};
  for(int c=0;c<8;c++){ Engine e=mk(); e.p[P_BARK]=c; e.p[P_CHANCE]=1000; e.p[P_INJECT]=1000; e.p[P_MIX]=1000; e.p[P_DISTANCE]=0; e.updateDistance(); float L,R; double s=0; for(int n=0;n<48000*3;n++){ e.process(0,0,L,R); s+=L*L+R*R; } printf("%s %.3f  ",nm[c],sqrt(s/(2*48000*3))); } printf("\n");
  printf("3) RANDOM: quante volte esce ciascun abbaio su 800 abbai\n   ");
  { Engine e=mk(); e.p[P_BARK]=8; e.p[P_CHANCE]=1000; e.p[P_INJECT]=1000; int cnt[8]={0}; for(int k=0;k<800;k++){ e.trigger(); if(e.active>=0) cnt[e.active]++; } for(int c=0;c<8;c++) printf("%s %d  ",nm[c],cnt[c]); printf("\n"); }
  printf("4) DISTANCE: un abbaio (BAU3) vicino e lontano: livello e brillantezza (centroide)\n");
  for(int d:{0,300,600,1000}){ Engine e=mk(); e.p[P_BARK]=2; e.p[P_CHANCE]=1000; e.p[P_INJECT]=1000; e.p[P_MIX]=1000; e.p[P_DISTANCE]=d; e.p[P_FEEDBACK]=0; e.p[P_TIME]=1023; e.updateDistance(); float L,R; std::vector<float> o; for(int n=0;n<32400+30000;n++){ e.process(0,0,L,R); if(n>=32400) o.push_back(0.5f*(L+R)); }
    double s=0; for(float x:o) s+=x*x; std::vector<float> seg(o.begin(),o.begin()+16384); static double r0=0; double r=sqrt(s/o.size()); if(d==0) r0=r; printf("   DISTANCE %4d: RMS %.3f (%+.1f dB)  centroide %4.0f Hz\n",d,r,20*log10(r/r0),cent(seg)); }
  printf("5) SPREAD: bilanciamento sinistra/destra di 8 richiami consecutivi (SPREAD 1000)\n   ");
  { Engine e=mk(); e.p[P_CHANCE]=1000; e.p[P_INJECT]=1000; e.p[P_MIX]=1000; e.p[P_SPREAD]=1000; e.p[P_FEEDBACK]=0; e.p[P_TIME]=1023; float L,R; for(int k=0;k<8;k++){ double l=0,r=0; for(int n=0;n<32400;n++){ e.process(0,0,L,R); l+=L*L; r+=R*R; } printf("%+.2f ",(r-l)/(r+l+1e-12)); } printf(" (-1 = tutto a sinistra, +1 = tutto a destra)\n"); }
  printf("6) Stabilita': FEEDBACK, INJECT, CHANCE al massimo, 20 s con ingresso\n");
  { Engine e=mk(); e.p[P_FEEDBACK]=1023; e.p[P_INJECT]=1000; e.p[P_CHANCE]=1000; e.p[P_MIX]=1000; float L,R; double pk=0; int nan=0; for(int n=0;n<48000*20;n++){ float x=(n%48000)<4800?0.5f*sinf(n*0.03f):0; e.process(x,x,L,R); if(!std::isfinite(L)||!std::isfinite(R)) nan++; pk=fmax(pk,fmax(fabs(L),fabs(R))); } printf("   picco %.2f, NaN %d\n",pk,nan); }
}
