#include <initializer_list>
#include <cstdio>
#include <cmath>
#include <vector>
#define private public
#include "../../logue-sdk/platform/nts-1_mkii/superphaser/phaser.h"
static std::vector<float> M(Phaser::kFloats);
static Phaser mk(){ Phaser p; p.init(M.data()); return p; }
static void run(Phaser&p,const std::vector<float>&x,std::vector<float>&L,std::vector<float>&R){ L.resize(x.size()); R.resize(x.size()); float in[128],out[128]; for(size_t i=0;i+64<=x.size();i+=64){ for(int k=0;k<64;k++) in[2*k]=in[2*k+1]=x[i+k]; p.process(in,out,64); for(int k=0;k<64;k++){ L[i+k]=out[2*k]; R[i+k]=out[2*k+1]; } } }
static std::vector<float> noise(float sec){ std::vector<float> v((int)(sec*48000)); uint32_t s=1; for(auto&x:v){ s^=s<<13; s^=s>>17; s^=s<<5; x=0.3f*((s>>8)/8388608.f-1); } return v; }
int main(){
  printf("1) Tacche dello spettro con LFO fermo (DEPTH 0, COLOR 0, ENSEMBLE 0, MIX 50%%): attese = stadi/2\n");
  for(int st=0;st<5;st++){ Phaser p=mk(); p.setParam(1,0); p.setParam(3,0); p.setParam(5,0); p.setParam(8,0); p.setParam(2,st); auto x=noise(3); std::vector<float> L,R; run(p,x,L,R);
    // risposta in frequenza (rapporto delle densita' spettrali) su 120 bande logaritmiche 30 Hz - 16 kHz
    std::vector<double> g; int N=32768; size_t a=48000; for(int b=0;b<120;b++){ double f=30*pow(16000/30.0,b/119.0); double rx=0,ix=0,ry=0,iy=0; for(int i=0;i<N;i++){ double w=0.5-0.5*cos(2*M_PI*i/N),c=cos(2*M_PI*f*i/48000),s=sin(2*M_PI*f*i/48000); rx+=x[a+i]*w*c; ix+=x[a+i]*w*s; ry+=L[a+i]*w*c; iy+=L[a+i]*w*s; } g.push_back(20*log10(sqrt((ry*ry+iy*iy)/(rx*rx+ix*ix))+1e-9)); }
    int notches=0; std::vector<int> pos; for(int b=1;b<119;b++) if(g[b]<g[b-1]&&g[b]<=g[b+1]&&g[b]<-15){ notches++; pos.push_back(b); }
    printf("   %2d stadi: %d tacche (attese %d) a",4+2*st,notches,(4+2*st)/2); for(int b:pos) printf(" %.0f",30*pow(16000/30.0,b/119.0)); printf(" Hz\n"); }
  printf("2) ENSEMBLE: differenza tra sinistra e destra (correlazione: 1 = mono, piu' bassa = piu' largo)\n");
  for(int e:{0,50,100}){ Phaser p=mk(); p.setParam(5,e); p.setParam(9,0); auto x=noise(3); std::vector<float> L,R; run(p,x,L,R); double sl=0,sr=0,slr=0; for(size_t i=48000;i<x.size();i++){ sl+=L[i]*L[i]; sr+=R[i]*R[i]; slr+=L[i]*R[i]; } printf("   ENSEMBLE %3d%% (solo ensemble, MIX 0): correlazione L/R %.2f\n",e,slr/sqrt(sl*sr)); }
  printf("3) Stabilita': COLOR 100%%, tutti gli stadi e le forme, ENSEMBLE e WARMTH al massimo\n");
  { double pk=0; int nan=0; for(int st=0;st<5;st++) for(int sh=0;sh<3;sh++){ Phaser p=mk(); p.setParam(2,st); p.setParam(3,100); p.setParam(7,sh); p.setParam(5,100); p.setParam(8,100); p.setParam(0,700); p.setParam(1,1023); auto x=noise(3); for(size_t i=0;i<x.size();i++) x[i]+=0.6f*sinf(i*0.01f); std::vector<float> L,R; run(p,x,L,R); for(size_t i=0;i<L.size();i++){ if(!std::isfinite(L[i])||!std::isfinite(R[i])) nan++; pk=fmax(pk,fmax(fabs(L[i]),fabs(R[i]))); } } printf("   picco %.2f, NaN %d\n",pk,nan); }
  printf("4) Livello rispetto all'ingresso (rumore), default\n");
  { Phaser p=mk(); auto x=noise(4); std::vector<float> L,R; run(p,x,L,R); double e=0,ex=0; for(size_t i=48000;i<x.size();i++){ e+=L[i]*L[i]; ex+=x[i]*x[i]; } printf("   %+.1f dB\n",10*log10(e/ex)); }
}
