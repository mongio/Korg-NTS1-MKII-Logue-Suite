#include <initializer_list>
#include <cstdio>
#include <cmath>
#include <vector>
#include <chrono>
#define private public
#include "../../logue-sdk/platform/nts-1_mkii/mee-nah/mee.h"
static std::vector<float> run(MeeNah&o,float note,int n){ std::vector<float> v; float b[64]; for(int k=0;k<n/64;k++){o.process(note,b,64); v.insert(v.end(),b,b+64);} return v;}
static float pitch(const std::vector<float>&x,int off){ // autocorrelazione normalizzata
  int N=9600; std::vector<double> r(2000,0); double mx=-1;
  for(int lag=30;lag<2000;lag++){ double s=0,e1=0,e2=0; for(int i=off;i<off+N;i++){ s+=x[i]*x[i+lag]; e1+=x[i]*x[i]; e2+=x[i+lag]*x[i+lag]; } r[lag]=s/sqrt(e1*e2+1e-12); if(r[lag]>mx) mx=r[lag]; }
  for(int lag=31;lag<1999;lag++) if(r[lag]>=0.9*mx && r[lag]>=r[lag-1] && r[lag]>=r[lag+1]){ double a=r[lag-1],b=r[lag],c=r[lag+1]; return 48000.0/(lag+0.5*(a-c)/(a-2*b+c)); }
  return 0; }
static float centroid(const std::vector<float>&x,int off){ // centroide spettrale grezzo via DFT 0-4 kHz
  int N=4096; double num=0,den=0; for(int k=1;k<342;k++){ double re=0,im=0; for(int i=0;i<N;i++){ double w=0.5-0.5*cos(2*M_PI*i/N); re+=x[off+i]*w*cos(2*M_PI*k*i/N); im-=x[off+i]*w*sin(2*M_PI*k*i/N);} double m=sqrt(re*re+im*im); num+=m*k*48000.0/N; den+=m; } return num/den; }
int main(){
  printf("PITCH KEY - vocale tenuta (frammento 2), freeze:\n");
  for(float note : {45.f,52.f,57.f,64.f,69.f,76.f}){
    MeeNah o; o.init(); o.setParam(1,0); o.setParam(6,0); o.setParam(9,0);
    o.setParam(0,(int)(1023*(kFrags[2].start+kFrags[2].len/2)/(float)MEE_NSAMP)); o.noteOn();
    auto v=run(o,note,48000); float want=440*powf(2,(note-69)/12);
    printf("  nota %3.0f: atteso %6.1f Hz, misurato %6.1f Hz, centroide %5.0f Hz\n",note,want,pitch(v,24000),centroid(v,24000));
  }
  printf("PITCH MELODY a C4 (altezza originale del frammento):\n");
  for(int f : {0,2,4}){
    MeeNah o; o.init(); o.setParam(1,0); o.setParam(4,1); o.setParam(6,0); o.setParam(9,0);
    o.setParam(0,(int)(1023*(kFrags[f].start+kFrags[f].len/2)/(float)MEE_NSAMP)); o.noteOn();
    auto v=run(o,60,48000); int fr=(kFrags[f].start+kFrags[f].len/2)/64; float src=6000.f/(kPeriod[fr]/3.f);
    printf("  frammento %d: originale ~%6.1f Hz, misurato %6.1f Hz\n",f,src,pitch(v,24000));
  }
  // livelli / NaN in tutte le combinazioni principali
  double pk=0; int nan=0; long N=0; double e=0;
  for(int m=0;m<4;m++) for(int pl=0;pl<3;pl++) for(int pm=0;pm<2;pm++) for(int fm : {0,12,24}) for(int sp : {0,815,1023}) for(float note : {36.f,60.f,84.f}) for(int ch:{0,100}){
    MeeNah o; o.init(); o.setParam(2,m); o.setParam(3,pl); o.setParam(4,pm); o.setParam(5,fm); o.setParam(1,sp); o.setParam(8,ch); o.setParam(9,100); o.setParam(6,60); o.setParam(7,50);
    o.noteOn(); auto v=run(o,note,24000); for(float x:v){ if(!std::isfinite(x)) nan++; pk=fmax(pk,fabs(x)); e+=x*x; N++; } }
  printf("Tutte le combinazioni: picco %.2f, RMS medio %.3f, NaN %d\n",pk,sqrt(e/N),nan);
  { MeeNah o; o.init(); o.setParam(8,100); o.setParam(2,3); o.setParam(6,100); auto t0=std::chrono::high_resolution_clock::now(); auto v=run(o,84,48000*4); auto t1=std::chrono::high_resolution_clock::now();
    printf("CPU (PC) caso peggiore (coro, nota alta): %.3f us/campione\n", std::chrono::duration<double,std::micro>(t1-t0).count()/v.size()); }
}
