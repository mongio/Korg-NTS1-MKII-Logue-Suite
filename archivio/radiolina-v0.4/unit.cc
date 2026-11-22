/*
 *  RADIOLINA v0.4 - interfaccia unit osc per NTS-1 mkII
 */
#include "unit_osc.h"
#include "utils/int_math.h"
#include "radiolina.h"
static Radiolina s_osc;
static int32_t s_cache[UNIT_OSC_MAX_PARAM_COUNT];
__unit_callback int8_t unit_init(const unit_runtime_desc_t *desc) {
  if (!desc) return k_unit_err_undef;
  if (desc->target != unit_header.target) return k_unit_err_target;
  if (!UNIT_API_IS_COMPAT(desc->api)) return k_unit_err_api_version;
  if (desc->samplerate != 48000) return k_unit_err_samplerate;
  if (desc->output_channels != 1) return k_unit_err_geometry;
  s_osc.init();
  for (int id = 0; id < UNIT_OSC_MAX_PARAM_COUNT; ++id) { s_cache[id] = (int32_t)unit_header.params[id].init; if (id < Radiolina::P_NUM) s_osc.setParam(id, s_cache[id]); }
  return k_unit_err_none;
}
__unit_callback void unit_teardown() {}
__unit_callback void unit_reset() {}
__unit_callback void unit_resume() {}
__unit_callback void unit_suspend() {}
__unit_callback void unit_render(const float *in, float *out, uint32_t frames) { (void)in; s_osc.process(out, frames); }
__unit_callback void unit_set_param_value(uint8_t id, int32_t value) {
  if (id >= Radiolina::P_NUM) return;
  value = clipminmaxi32(unit_header.params[id].min, value, unit_header.params[id].max);
  s_cache[id] = value; s_osc.setParam(id, value);
}
__unit_callback int32_t unit_get_param_value(uint8_t id) { return id < UNIT_OSC_MAX_PARAM_COUNT ? s_cache[id] : 0; }
__unit_callback const char *unit_get_param_str_value(uint8_t id, int32_t value) { return s_osc.paramStr(id, value); }
__unit_callback void unit_note_on(uint8_t note, uint8_t velo) { (void)velo; s_osc.noteOn(note); }
__unit_callback void unit_note_off(uint8_t note) { (void)note; }
__unit_callback void unit_all_note_off() {}
__unit_callback void unit_set_tempo(uint32_t tempo) { s_osc.setTempo((float)(tempo >> 16) + (float)(tempo & 0xFFFF) / 65536.f); }
__unit_callback void unit_tempo_4ppqn_tick(uint32_t counter) { (void)counter; }
__unit_callback void unit_pitch_bend(uint16_t bend) { (void)bend; }
__unit_callback void unit_channel_pressure(uint8_t press) { (void)press; }
__unit_callback void unit_aftertouch(uint8_t note, uint8_t press) { (void)note; (void)press; }
