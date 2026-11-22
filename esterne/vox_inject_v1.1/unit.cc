#include "unit_delfx.h"
#include "utils/int_math.h"
#include "dsp_core.h"
namespace {static vox::Engine engine;}
__unit_callback int8_t unit_init(const unit_runtime_desc_t *d){
 if(!d)return k_unit_err_undef;
 if(d->target!=unit_header.target)return k_unit_err_target;
 if(!UNIT_API_IS_COMPAT(d->api))return k_unit_err_api_version;
 if(d->samplerate!=48000)return k_unit_err_samplerate;
 if(d->input_channels!=2||d->output_channels!=2)return k_unit_err_geometry;
 // il buffer del delay (32 KB) va nella memoria esterna: nella RAM interna dell'unita' non ci sta
 if(!d->hooks.sdram_alloc)return k_unit_err_memory;
 int16_t *buf=(int16_t*)d->hooks.sdram_alloc(vox::BUF*sizeof(int16_t));
 if(!buf)return k_unit_err_memory;
 engine.setBuffer(buf);
 for(int i=0;i<vox::P_NUM;i++)engine.p[i]=unit_header.params[i].init;
 engine.reset();return k_unit_err_none;
}
__unit_callback void unit_teardown(){}
__unit_callback void unit_reset(){engine.reset();}
__unit_callback void unit_resume(){}
__unit_callback void unit_suspend(){}
__unit_callback void unit_render(const float *in,float *out,uint32_t frames){
 for(uint32_t n=0;n<frames;n++)engine.process(in[2*n],in[2*n+1],out[2*n],out[2*n+1]);
}
__unit_callback void unit_set_param_value(uint8_t id,int32_t v){if(id<vox::P_NUM)engine.p[id]=clipminmaxi32(unit_header.params[id].min,v,unit_header.params[id].max);}
__unit_callback int32_t unit_get_param_value(uint8_t id){return id<vox::P_NUM?engine.p[id]:0;}
__unit_callback const char *unit_get_param_str_value(uint8_t,int32_t){return nullptr;}
__unit_callback void unit_set_tempo(uint32_t){}
__unit_callback void unit_tempo_4ppqn_tick(uint32_t){}
