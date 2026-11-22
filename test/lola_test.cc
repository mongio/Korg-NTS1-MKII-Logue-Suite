#include <initializer_list>
#include <cstdio>
#include <cmath>
#include <vector>
#include <chrono>
#define private public
#include "../../logue-sdk/platform/nts-1_mkii/lola/lola.h"
static std::vector<float> run(Lola&l,float note,float sec){ std::vector<float> v; float b[64]; for(int k=0;k<(int)(sec*48000)/64;k++){ l.process(note,b,64); v.insert(v.end(),b,b+64);} return v; }
static float f0(const std::vector<float>&x,size_t a,int N){ double mx=-1; int best=0; std::vector<double> r(1200);
  for(int lag=30;lag<1100;lag++){ double s=0,e1=0,e2=0; for(int i=0;i<N;i++){ s+=x[a+i]*x[a+i+lag]; e1+=x[a+i]*x[a+i]; e2+=x[a+i+lag]*x[a+i+lag]; } r[lag]=s/sqrt(e1*e2+1e-12); if(r[lag]>mx){mx=r[lag];} }
  for(int lag=31;lag<1099;lag++) if(r[lag]>=0.85*mx && r[lag]>=r[lag-1]&&r[lag]>=r[lag+1]){ best=lag; break; } return best? 48000.f/best : 0; }
static void bursts(const std::vector<float>&v, int &count, float &first_len){ count=0; first_len=0; bool on=false; size_t st=0; double e=0;
  for(size_t i=0;i+480<=v.size();i+=480){ double s=0; for(size_t k=i;k<i+480;k++) s+=v[k]*v[k]; s=sqrt(s/480); if(!on && s>0.02){ on=true; st=i; count++; } else if(on && s<0.008){ on=false; if(count==1) first_len=(i-st)/48000.f; } } }
Lola mk(int call){ Lola l; l.init(); l.setParam(2,call); l.setParam(6,0); l.setParam(9,0); return l; }
int main(){
  const char* cn[]={"BARK","YIP","ARF","GROWL","WHINE","HOWL","MIX"};
  printf("1) Ogni verso (SIZE 25%%, NATURAL, C4): durata, altezza nel tempo, picco\n");
  for(int c=0;c<6;c++){ Lola l=mk(c); l.noteOn(60); auto v=run(l,60,2.5f); l.noteOff(60); int cnt; float len; bursts(v,cnt,len);
    double pk=0; int nan=0; for(float x:v){ if(!std::isfinite(x)) nan++; pk=fmax(pk,fabs(x)); }
    float dur=l.cur_.dur; printf("   %-6s durata %.2f s, colpi %d, picco %.2f, NaN %d | Hz:",cn[c],c==2?len:dur,cnt,pk,nan);
    for(float fr : {0.12f,0.3f,0.6f,0.85f}){ size_t a=(size_t)(fr*dur*48000); if(c==2) a=(size_t)(fr*0.08f*48000); if(a+3000<v.size()) printf(" %4.0f",f0(v,a,(int)fminf(1400,dur*48000*0.15f+400))); } printf("\n"); }
  printf("2) Taglia (SIZE) e altezza naturale del BARK a C4 (picco del bau)\n   ");
  for(int s : {0,250,500,1023}){ Lola l=mk(0); l.setParam(0,s); l.noteOn(60); auto v=run(l,60,0.3f); printf("SIZE %4d: %4.0f Hz   ",s,f0(v,(size_t)(0.035*48000),900)); } printf("\n");
  printf("3) PITCH KEY: il BARK segue la nota (picco = nota +2 semitoni)\n   ");
  for(int note : {48,60,72}){ Lola l=mk(0); l.setParam(4,0); l.noteOn(note); auto v=run(l,note,0.3f); printf("nota %d: %4.0f Hz (atteso ~%4.0f)   ",note,f0(v,(size_t)(0.035*48000),900),440*powf(2,(note+2-69)/12.f)); } printf("\n");
  printf("4) REPEAT con tasto tenuto 4 s\n");
  for(int rp : {10,20,43}){ Lola l=mk(0); l.setTempo(120); l.setParam(3,rp); l.noteOn(60); auto v=run(l,60,4.f); int cnt; float len; bursts(v,cnt,len); printf("   REPEAT %-5s -> %d bau in 4 s\n", l.paramStr(3,rp), cnt); }
  printf("5) PANT: fiatone tra i bau con tasto tenuto, silenzio dopo il rilascio\n");
  { Lola l=mk(0); l.setParam(8,80); l.noteOn(60); auto v=run(l,60,1.5f); double e=0; for(size_t i=24000;i<72000;i++) e+=v[i]*v[i]; l.noteOff(60); auto w=run(l,60,1.f); double e2=0; for(float x:w) e2+=x*x;
    printf("   RMS tasto tenuto (dopo il bau) %.3f, dopo il rilascio %.4f\n",sqrt(e/48000),sqrt(e2/w.size())); }
  printf("6) CRACK 100%%: quanti bau hanno la stecca all'ottava (su 40)\n");
  { int n=0; for(int k=0;k<40;k++){ Lola l=mk(0); l.rng_=1234+k*7919; l.setParam(9,100); l.noteOn(60); if(l.crack_at_>=0) n++; } printf("   %d/40\n",n); }
  printf("7) Livelli: picco e RMS per verso e taglia, ROUGH 0/100\n");
  for(int c=0;c<6;c++){ double pk=0,rm=0; int nan=0; for(int s:{0,250,1023}) for(int r:{0,1023}){ Lola l=mk(c); l.setParam(0,s); l.setParam(1,r); l.noteOn(60); auto v=run(l,60,2.f); double e=0; int act=0; for(float x:v){ if(!std::isfinite(x)) nan++; pk=fmax(pk,fabs(x)); if(fabs(x)>0.001){ e+=x*x; act++; } } rm=fmax(rm,sqrt(e/fmax(1,act))); }
    printf("   %-6s picco max %.2f, RMS max %.2f, NaN %d\n",cn[c],pk,rm,nan); }
  { Lola l=mk(5); l.setParam(3,30); l.setParam(8,50); l.noteOn(60); auto t0=std::chrono::high_resolution_clock::now(); run(l,72,4.f); auto t1=std::chrono::high_resolution_clock::now(); printf("8) CPU (PC): %.3f us/campione\n",std::chrono::duration<double,std::micro>(t1-t0).count()/(4*48000)); }
}
