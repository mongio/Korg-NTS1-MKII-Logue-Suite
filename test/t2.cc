#include <initializer_list>
#include <cstdio>
#include <cmath>
#include <vector>
#include "../../logue-sdk/platform/nts-1_mkii/mee-nah/mee.h"
int main(){ float b[64];
  for(float note : {36.f,48.f,60.f,72.f,84.f}) for(int fm : {0,12,24}) for(int tp : {0,100}) for(int ch:{0,100}){
    double pk=0,e=0; long clip=0,N=0;
    for(int m=0;m<4;m++) for(int sp:{0,815}){ MeeNah o; o.init(); o.setParam(2,m); o.setParam(5,fm); o.setParam(1,sp); o.setParam(8,ch); o.setParam(9,tp); o.setParam(6,30); o.noteOn();
      for(int k=0;k<24000/64;k++){ o.process(note,b,64); for(float x:b){ pk=fmax(pk,fabs(x)); e+=x*x; if(fabs(x)>0.95) clip++; N++; } } }
    if(clip>0 || note==60) printf("nota %2.0f form %2d tape %3d coro %3d: picco %.2f RMS %.3f clip %.2f%%\n",note,fm-12,tp,ch,pk,sqrt(e/N),100.0*clip/N);
  }
}
