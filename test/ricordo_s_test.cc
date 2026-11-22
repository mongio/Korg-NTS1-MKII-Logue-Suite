#include <initializer_list>
#include <cstdio>
#include <cmath>
#include <vector>
#include <chrono>
#define private public
#include "ricordo_s.h"
static std::vector<float> BUF(RicordoS::kBufFloats);
static RicordoS mk(){ RicordoS r; r.init(BUF.data()); r.setParam(RicordoS::P_MIX,1000); return r; }
static std::vector<float> run(RicordoS&r,const std::vector<float>&x){ std::vector<float> o(x.size()); float in[128],out[128];
  for(size_t i=0;i+64<=x.size();i+=64){ for(int k=0;k<64;k++) in[2*k]=in[2*k+1]=x[i+k]; r.process(in,out,64); for(int k=0;k<64;k++) o[i+k]=0.5f*(out[2*k]+out[2*k+1]); } return o; }
static float t30(const std::vector<float>&h){ std::vector<double> e(h.size()); double acc=0; for(int i=h.size()-1;i>=0;i--){ acc+=(double)h[i]*h[i]; e[i]=acc; }
  double e0=e[0]; int i5=-1,i35=-1; for(size_t i=0;i<e.size();i++){ double db=10*log10(e[i]/e0+1e-30); if(i5<0&&db<-5) i5=i; if(i35<0&&db<-35){i35=i;break;} } return (i35<0||i5<0)?-1:2.0*(i35-i5)/48000.0; }
static double band(const std::vector<float>&v,size_t a,double f){ double re=0,im=0; int N=8192; for(int i=0;i<N;i++){ double w=0.5-0.5*cos(2*M_PI*i/N); re+=v[a+i]*w*cos(2*M_PI*f*i/48000); im+=v[a+i]*w*sin(2*M_PI*f*i/48000);} return sqrt(re*re+im*im); }
static double rms(const std::vector<float>&v,size_t a,size_t b){ double e=0; for(size_t i=a;i<b;i++) e+=v[i]*v[i]; return sqrt(e/(b-a)); }
int main(){
  printf("1) DECAY fino a 100 s (AGE 0, HISS 0, TONE 100)\n");
  for(float target : {1.f,5.f,20.f,60.f,100.f}){ int d=(int)lround(log2(target/0.5)/7.644*1023); if(d>1023)d=1023; RicordoS r=mk(); r.setParam(0,d); r.setParam(1,0); r.setParam(8,0); r.setParam(5,100);
    float rt=0.5f*powf(2,d/1023.f*7.644f); std::vector<float> x((int)(48000*fmax(2.5,rt*0.75)),0.f); x[0]=1; auto h=run(r,x); printf("   DECAY %4d: impostato %6.1f s, misurato %6.1f s\n",d,rt,t30(h)); }
  printf("2) FREEZE a livello costante: RMS 1 s dopo il freeze e 20 s dopo\n");
  const char* en[]={"NASTRO","VINILE","RADIO","CERA","TUBO"};
  for(int e=0;e<5;e++) for(int age : {0,460,1023}){ RicordoS r=mk(); r.setParam(3,e); r.setParam(1,age); r.setParam(0,600);
    std::vector<float> x(48000,0.f); for(int i=0;i<24000;i++) x[i]=0.4f*sinf(2*M_PI*330*i/48000.f)+0.2f*sinf(2*M_PI*523*i/48000.f); run(r,x); r.setParam(10,1);
    auto o=run(r,std::vector<float>(48000*21,0.f)); double a=rms(o,48000,96000), b=rms(o,o.size()-48000,o.size()); double pk=0; int nan=0; for(float v:o){ if(!std::isfinite(v)) nan++; pk=fmax(pk,fabs(v)); }
    printf("   %-6s AGE %4d: %.3f -> %.3f (%+.1f dB) picco %.2f NaN %d\n",en[e],age,a,b,20*log10(b/a),pk,nan); }
  printf("3) TUBO: shimmer, energia all'ottava sopra (2f) e due ottave (4f) rispetto alla fondamentale, nel tempo\n");
  for(int age : {0,500,1023}){ RicordoS r=mk(); r.setParam(3,4); r.setParam(1,age); r.setParam(0,750); r.setParam(5,100); std::vector<float> x(48000*6,0.f); for(int i=0;i<12000;i++) x[i]=0.4f*sinf(2*M_PI*440*i/48000.f); auto o=run(r,x);
    printf("   AGE %4d:",age); for(float t:{0.5f,2.f,4.f}){ size_t a=(size_t)(t*48000); double f1=band(o,a,440),f2=band(o,a,880),f4=band(o,a,1760); printf("  t=%.1fs 2f %+5.1f 4f %+5.1f RMS %.3f",t,20*log10(f2/f1),20*log10(f4/f1),rms(o,a,a+8192)); } printf("\n"); }
  printf("4) Stabilita': TUBO, AGE e DECAY al massimo, 40 s\n");
  { RicordoS r=mk(); r.setParam(3,4); r.setParam(1,1023); r.setParam(0,1023); r.setParam(5,100); std::vector<float> x(48000*40,0.f); for(int i=0;i<48000;i++) x[i]=0.8f*sinf(2*M_PI*220*i/48000.f); auto o=run(r,x);
    double pk=0; int nan=0; for(float v:o){ if(!std::isfinite(v)) nan++; pk=fmax(pk,fabs(v)); } printf("   picco %.2f, RMS a 5 s %.3f, a 39 s %.3f, NaN %d\n",pk,rms(o,5*48000,6*48000),rms(o,39*48000,40*48000),nan); }
  printf("5) LOWCUT: attenuazione della coda a 80 Hz e a 2 kHz\n");
  for(int lc : {0,30,60,100}){ RicordoS r=mk(); r.setParam(7,lc); r.setParam(1,0); r.setParam(8,0); std::vector<float> x(48000*2,0.f); for(int i=0;i<24000;i++) x[i]=0.3f*sinf(2*M_PI*80*i/48000.f)+0.3f*sinf(2*M_PI*2000*i/48000.f); auto o=run(r,x);
    static double r80,r2k; double a=band(o,36000,80), b=band(o,36000,2000); if(lc==0){r80=a;r2k=b;} printf("   LOWCUT %3d%% (taglio %4.0f Hz): 80 Hz %+5.1f dB, 2 kHz %+5.1f dB\n",lc,20*powf(2,lc/100.f*5.9f),20*log10(a/r80),20*log10(b/r2k)); }
  { RicordoS r=mk(); r.setParam(3,4); r.setParam(1,1023); for(int p:{6,8,9}) r.setParam(p,100); std::vector<float> x(48000*4); for(size_t i=0;i<x.size();i++) x[i]=0.3f*sinf(i*0.05f); auto t0=std::chrono::high_resolution_clock::now(); run(r,x); auto t1=std::chrono::high_resolution_clock::now();
    printf("6) CPU (PC), TUBO con tutto attivo: %.3f us/campione\n", std::chrono::duration<double,std::micro>(t1-t0).count()/x.size()); }
}
