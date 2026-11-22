#include "osc.h"
#include "unit_osc.h"
#include "utils/int_math.h"
static Osc osc; static int32_t val[UNIT_OSC_MAX_PARAM_COUNT]; static const unit_runtime_osc_context_t *ctx;
// EDIT is a host menu parameter (0..5), not a knob mode.
// VALUE is the only editor. Physical A/B always control ALT/SHAPE.
static int editSelection=0;
// UI only: constant numeric labels, no formatting or allocation on audio thread.
static char numericLabels[101][4];
static void initNumericLabels(){
 for(int n=0;n<=100;n++){
  int i=0;
  if(n>=100)numericLabels[n][i++]='1',numericLabels[n][i++]='0',numericLabels[n][i++]='0';
  else {if(n>=10)numericLabels[n][i++]=(char)('0'+n/10);numericLabels[n][i++]=(char)('0'+n%10);}
  numericLabels[n][i]=0;
 }
}
static int clampi(int v,int lo,int hi){return v<lo?lo:(v>hi?hi:v);}
static int editMax(int e){return e==0?17:e==1?6:e==4?2:e==5?1:e==10?3:e==12?16:100;}
static int toHost(int actual,int e){const int max=editMax(e);return e==12?((actual-2)*100+7)/14:((actual*100+max/2)/max);}
static void applyValue(int hostValue){
 const int max=editMax(editSelection);
 const int mapped=(editSelection==12)?(2+(hostValue*14+50)/100):((hostValue*max+50)/100);
 // RESET is intentionally only fired on a deliberate VALUE write.
 osc.setParameter(9,mapped);
 val[9]=hostValue;
}

#define Q31F 4.65661287307739e-10f
__unit_callback int8_t unit_init(const unit_runtime_desc_t *d){if(!d)return k_unit_err_undef;if(d->target!=unit_header.target)return k_unit_err_target;if(!UNIT_API_IS_COMPAT(d->api))return k_unit_err_api_version;if(d->samplerate!=48000)return k_unit_err_samplerate;if(d->input_channels!=2||d->output_channels!=1)return k_unit_err_geometry;ctx=(const unit_runtime_osc_context_t*)d->hooks.runtime_context;initNumericLabels();osc.reset();editSelection=0;for(int i=0;i<UNIT_OSC_MAX_PARAM_COUNT;i++){val[i]=unit_header.params[i].init;osc.setParameter(i,val[i]);}return k_unit_err_none;}
__unit_callback void unit_teardown(){osc.teardown();} __unit_callback void unit_reset(){osc.reset();}
__unit_callback void unit_resume(){osc.resume();} __unit_callback void unit_suspend(){osc.suspend();}
__unit_callback void unit_render(const float *in,float*out,uint32_t frames){osc.setPitch(osc_w0f_for_note(ctx->pitch>>8,ctx->pitch&255));osc.setShapeLfo((float)ctx->shape_lfo*Q31F);osc.process(in,out,frames);}
__unit_callback void unit_set_param_value(uint8_t id,int32_t v){
 if(id>=unit_header.num_params)return;
 v=clipminmaxi32(unit_header.params[id].min,v,unit_header.params[id].max);
 if(id==8){
  editSelection=v;
  osc.selectEdit(v);
  val[8]=v;
  // VALUE's logical position follows the saved value of the newly selected edit item.
  // Do not apply it: doing so would overwrite the selected setting or clear notes.
  val[9]=toHost(osc.editValue(),editSelection);
  return;
 }
 if(id==9){applyValue(v);return;}
 val[id]=v;osc.setParameter(id,v);
}
__unit_callback int32_t unit_get_param_value(uint8_t id){return id<unit_header.num_params?val[id]:0;}
__unit_callback const char *unit_get_param_str_value(uint8_t id,int32_t v){
 if(id==9 && (editSelection==2 || editSelection==3 || (editSelection>=6 && editSelection!=10))){
  return numericLabels[clampi(editSelection==12?2+(v*14+50)/100:v,0,100)];
 }
 return osc.getParameterStrValue(id,v);
}
__unit_callback void unit_note_on(uint8_t n,uint8_t v){osc.noteOn(n,v);} __unit_callback void unit_note_off(uint8_t n){osc.noteOff(n);} __unit_callback void unit_all_note_off(){osc.allNoteOff();}
__unit_callback void unit_set_tempo(uint32_t t){osc.setTempo((t>>16)+(t&0xffff)/65536.f);} __unit_callback void unit_tempo_4ppqn_tick(uint32_t c){osc.tempo4ppqnTick(c);}
__unit_callback void unit_pitch_bend(uint16_t b){osc.pitchBend(b);} __unit_callback void unit_channel_pressure(uint8_t p){osc.channelPressure(p);} __unit_callback void unit_aftertouch(uint8_t n,uint8_t p){osc.aftertouch(n,p);}
