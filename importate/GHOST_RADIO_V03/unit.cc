#include "osc.h"
#include "unit_osc.h"
#include "utils/int_math.h"
static Osc osc; static int32_t val[UNIT_OSC_MAX_PARAM_COUNT]; static const unit_runtime_osc_context_t *ctx;
#define Q31F 4.65661287307739e-10f
__unit_callback int8_t unit_init(const unit_runtime_desc_t *d){if(!d)return k_unit_err_undef;if(d->target!=unit_header.target)return k_unit_err_target;if(!UNIT_API_IS_COMPAT(d->api))return k_unit_err_api_version;if(d->samplerate!=48000)return k_unit_err_samplerate;if(d->input_channels!=2||d->output_channels!=1)return k_unit_err_geometry;ctx=(const unit_runtime_osc_context_t*)d->hooks.runtime_context;for(int i=0;i<UNIT_OSC_MAX_PARAM_COUNT;i++){val[i]=unit_header.params[i].init;osc.setParameter(i,val[i]);}return k_unit_err_none;}
__unit_callback void unit_teardown(){osc.teardown();} __unit_callback void unit_reset(){osc.reset();}
__unit_callback void unit_resume(){osc.resume();} __unit_callback void unit_suspend(){osc.suspend();}
__unit_callback void unit_render(const float *in,float*out,uint32_t frames){osc.setPitch(osc_w0f_for_note(ctx->pitch>>8,ctx->pitch&255));osc.setShapeLfo((float)ctx->shape_lfo*Q31F);osc.process(in,out,frames);}
__unit_callback void unit_set_param_value(uint8_t id,int32_t v){v=clipminmaxi32(unit_header.params[id].min,v,unit_header.params[id].max);val[id]=v;osc.setParameter(id,v);}
__unit_callback int32_t unit_get_param_value(uint8_t id){return val[id];}
__unit_callback const char *unit_get_param_str_value(uint8_t id,int32_t v){return osc.getParameterStrValue(id,v);}
__unit_callback void unit_note_on(uint8_t n,uint8_t v){osc.noteOn(n,v);} __unit_callback void unit_note_off(uint8_t n){osc.noteOff(n);} __unit_callback void unit_all_note_off(){osc.allNoteOff();}
__unit_callback void unit_set_tempo(uint32_t t){osc.setTempo((t>>16)+(t&0xffff)/65536.f);} __unit_callback void unit_tempo_4ppqn_tick(uint32_t c){osc.tempo4ppqnTick(c);}
__unit_callback void unit_pitch_bend(uint16_t b){osc.pitchBend(b);} __unit_callback void unit_channel_pressure(uint8_t p){osc.channelPressure(p);} __unit_callback void unit_aftertouch(uint8_t n,uint8_t p){osc.aftertouch(n,p);}
