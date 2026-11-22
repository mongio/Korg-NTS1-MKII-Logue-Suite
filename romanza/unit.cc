/*
 *  File: unit.cc
 *  ROMANZA v0.3 - interfaccia unit per NTS-1 mkII (basata sul template KORG, BSD-3)
 */
#include "unit_osc.h"
#include "utils/int_math.h"
#include "mee.h"

static MeeNah s_osc;
static int32_t cached_values[UNIT_OSC_MAX_PARAM_COUNT];
static const unit_runtime_osc_context_t *context;

#define q31_to_f32_c 4.65661287307739e-010f
#define q31_to_f32(q) ((float)(q) * q31_to_f32_c)

__unit_callback int8_t unit_init(const unit_runtime_desc_t *desc) {
  if (!desc) return k_unit_err_undef;
  if (desc->target != unit_header.target) return k_unit_err_target;
  if (!UNIT_API_IS_COMPAT(desc->api)) return k_unit_err_api_version;
  if (desc->samplerate != 48000) return k_unit_err_samplerate;
  if (desc->input_channels != 2 || desc->output_channels != 1) return k_unit_err_geometry;

  context = static_cast<const unit_runtime_osc_context_t *>(desc->hooks.runtime_context);
  s_osc.init();
  for (int id = 0; id < UNIT_OSC_MAX_PARAM_COUNT; ++id) {
    cached_values[id] = static_cast<int32_t>(unit_header.params[id].init);
    if (id < MeeNah::P_NUM) s_osc.setParam(id, cached_values[id]);
  }
  return k_unit_err_none;
}

__unit_callback void unit_teardown() {}
__unit_callback void unit_reset() {}
__unit_callback void unit_resume() {}
__unit_callback void unit_suspend() {}

__unit_callback void unit_render(const float *in, float *out, uint32_t frames) {
  (void)in;
  const float base = (float)(context->pitch >> 8) + (float)(context->pitch & 0xFF) * (1.f / 256.f);
  s_osc.setLfo(q31_to_f32(context->shape_lfo));
  s_osc.process(base, out, frames);
}

__unit_callback void unit_set_param_value(uint8_t id, int32_t value) {
  if (id >= MeeNah::P_NUM) return;
  value = clipminmaxi32(unit_header.params[id].min, value, unit_header.params[id].max);
  cached_values[id] = value;
  s_osc.setParam(id, value);
}

__unit_callback int32_t unit_get_param_value(uint8_t id) {
  if (id >= UNIT_OSC_MAX_PARAM_COUNT) return 0;
  return cached_values[id];
}

__unit_callback const char *unit_get_param_str_value(uint8_t id, int32_t value) {
  if (id >= MeeNah::P_NUM) return nullptr;
  value = clipminmaxi32(unit_header.params[id].min, value, unit_header.params[id].max);
  return s_osc.paramStr(id, value);
}

__unit_callback void unit_note_on(uint8_t note, uint8_t velo) { (void)velo; s_osc.noteOn(note); }
__unit_callback void unit_note_off(uint8_t note) { s_osc.noteOff(note); }
__unit_callback void unit_all_note_off() { s_osc.allNotesOff(); }

__unit_callback void unit_set_tempo(uint32_t tempo) {
  (void)tempo;
}

__unit_callback void unit_tempo_4ppqn_tick(uint32_t counter) { (void)counter; }
__unit_callback void unit_pitch_bend(uint16_t bend) { (void)bend; }
__unit_callback void unit_channel_pressure(uint8_t press) { (void)press; }
__unit_callback void unit_aftertouch(uint8_t note, uint8_t press) { (void)note; (void)press; }
