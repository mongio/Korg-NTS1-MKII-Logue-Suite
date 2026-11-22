#include <initializer_list>
#include <cstdio>
#include <cmath>
#include <vector>
#include <chrono>
#define private public
#include "../../logue-sdk/platform/nts-1_mkii/radio/radio.h"
static std::vector<float> G(1<<15);
static std::vector<float> run(Radio&r, std::vector<float> x){ std::vector<float> o(x.size()); float in[128],out[128];
  for(size_t i=0;i+64<=x.size();i+=64){ for(int k=0;k<64;k++) in[2*k]=in[2*k+1]=x[i+k]; r.process(in,out,64); for(int k=0;k<64;k++) o[i+k]=out[2*k]; } return o; }
static std::vector<float> sine(float f,float sec,float a=0.5f){ std::vector<float> v((int)(sec*48000)); for(size_t i=0;i<v.size();i++) v[i]=a*sinf(2*M_PI*f*i/48000); return v; }
static double rms(const std::vector<float>&v,size_t a,size_t b){ double e=0; for(size_t i=a;i<b;i++) e+=v[i]*v[i]; return sqrt(e/(b-a)); }
static float peakfreq(const std::vector<float>&v,size_t off,float f0,float f1){ int N=16384; double best=0; float bf=0; for(float f=f0;f<=f1;f+=2){ double re=0,im=0; for(int i=0;i<N;i++){ double w=0.5-0.5*cos(2*M_PI*i/N); re+=v[off+i]*w*cos(2*M_PI*f*i/48000); im+=v[off+i]*w*sin(2*M_PI*f*i/48000);} double m=re*re+im*im; if(m>best){best=m;bf=f;} } return bf; }
Radio mk(int band){ Radio r; r.init(G.data(),1<<15); r.setParam(2,band); for(int p:{1,3,4,5,6,7,8}) r.setParam(p,0); r.setParam(9,100); return r; }
int main(){
  const char* bn[]={"FM","AM","SW","POCKET"};
  printf("1) In sintonia, tutto il resto a zero: guadagno su seno 1 kHz (deve essere ~1)\n");
  for(int b=0;b<4;b++){ Radio r=mk(b); auto x=sine(1000,2); auto o=run(r,x); printf("   %-6s guadagno %.2f\n",bn[b], rms(o,48000,96000)/rms(x,48000,96000)); }
  printf("2) Sintonia vs segnale (seno 1 kHz) e rumore (ingresso muto), banda AM, STATIC 60%%\n   TUNE  segnale  statico\n");
  for(int t : {512,560,620,700,800,860,900}){ Radio r=mk(1); r.setParam(5,60); r.setParam(0,t); r.dial_=t/1023.f; auto o=run(r,sine(1000,1.5)); Radio q=mk(1); q.setParam(5,60); q.setParam(0,t); q.dial_=t/1023.f; auto n=run(q,std::vector<float>(72000,0.f)); printf("   %4d   %.3f    %.3f\n",t,rms(o,24000,72000),rms(n,24000,72000)); }
  printf("3) Fischio di battimento AM (ingresso muto), frequenza attesa 60 + 9000*|scarto|\n");
  for(int t : {560,600,650}){ Radio r=mk(1); r.setParam(5,50); r.setParam(0,t); r.dial_=t/1023.f; auto o=run(r,std::vector<float>(72000,0.f)); float d=fabs(t/1023.f-0.5f); printf("   TUNE %d: atteso %5.0f Hz, misurato %5.0f Hz\n",t,60+9000*d,peakfreq(o,40000,50,2500)); }
  printf("4) SW, banda laterale stonata: seno 1 kHz spostato di scarto*2600 Hz\n");
  for(int t : {512,530,560,490}){ Radio r=mk(2); r.setParam(0,t); r.dial_=t/1023.f; auto o=run(r,sine(1000,1.5)); float sh=(t/1023.f-0.5f)*2600; if(sh>450)sh=450; if(sh<-450)sh=-450; printf("   TUNE %d: atteso %5.0f Hz, misurato %5.0f Hz\n",t,1000+sh,peakfreq(o,40000,400,1600)); }
  printf("5) Deriva e ricerca: escursione della sintonia in 20 s\n");
  for(int mode=0;mode<2;mode++){ Radio r=mk(1); if(mode==0) r.setParam(4,100); else r.setParam(8,60); float mn=1,mx=0; float in[128]={0},out[128]; for(int k=0;k<20*48000/64;k++){ r.process(in,out,64); mn=fminf(mn,r.dial_); mx=fmaxf(mx,r.dial_);} printf("   %s: sintonia tra %.2f e %.2f\n", mode?"SEEK 60%%":"DRIFT 100%%", mn, mx); }
  printf("6) Tutte le bande, tutto al massimo, sintonia ovunque: picco / DC / NaN\n");
  double pk=0,dc=0; int nan=0;
  for(int b=0;b<4;b++) for(int t : {0,180,350,512,700,860,1023}) { Radio r; r.init(G.data(),1<<15); r.setParam(2,b); r.setParam(0,t); r.dial_=t/1023.f; for(int p:{1}) r.setParam(p,1023); for(int p:{3,4,5,6,7}) r.setParam(p,100); r.setParam(9,100);
    auto x=sine(220,2,0.9f); for(size_t i=0;i<x.size();i++) x[i]+=0.3f*sinf(2*M_PI*3300*i/48000); auto o=run(r,x); double s=0; for(float v:o){ if(!std::isfinite(v)) nan++; pk=fmax(pk,fabs(v)); s+=v; } dc=fmax(dc,fabs(s/o.size())); }
  printf("   picco %.2f, DC max %.3f, NaN %d\n",pk,dc,nan);
  { Radio r=mk(2); for(int p:{3,4,5,6,7,8}) r.setParam(p,100); auto x=sine(300,4); auto t0=std::chrono::high_resolution_clock::now(); run(r,x); auto t1=std::chrono::high_resolution_clock::now(); printf("7) CPU (PC), SW con tutto attivo: %.3f us/campione\n", std::chrono::duration<double,std::micro>(t1-t0).count()/x.size()); }
}
