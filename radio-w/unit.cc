/*
 *  File: unit.cc
 *  RADIO-W v0.2 - interfaccia unit modfx per NTS-1 mkII (basata sul template KORG, BSD-3)
 */
#include "unit_modfx.h"
#include "utils/int_math.h"
#include "radio.h"

static Radio s_fx;
static int32_t cached_values[UNIT_MODFX_MAX_PARAM_COUNT];

__unit_callback int8_t unit_init(const unit_runtime_desc_t *desc) {
  if (!desc) return k_unit_err_undef;
  if (desc->target != unit_header.target) return k_unit_err_target;
  if (!UNIT_API_IS_COMPAT(desc->api)) return k_unit_err_api_version;
  if (desc->samplerate != 48000) return k_unit_err_samplerate;
  if (desc->input_channels != 2 || desc->output_channels != 2) return k_unit_err_geometry;

  // buffer della stazione fantasma: 2^15 campioni (0.68 s), ripiego 2^14; senza memoria funziona lo stesso
  float *buf = nullptr; uint32_t size = 0;
  if (desc->hooks.sdram_alloc) {
    for (uint32_t sz = (1u << 15); sz >= (1u << 14); sz >>= 1) {
      if (desc->hooks.sdram_avail && desc->hooks.sdram_avail() < sz * sizeof(float) + 64) continue;
      buf = (float *)desc->hooks.sdram_alloc(sz * sizeof(float));
      if (buf) { size = sz; break; }
    }
  }
  s_fx.init(buf, size ? size : 1u);
  for (int id = 0; id < UNIT_MODFX_MAX_PARAM_COUNT; ++id) {
    cached_values[id] = static_cast<int32_t>(unit_header.params[id].init);
    if (id < Radio::P_NUM) s_fx.setParam(id, cached_values[id]);
  }
  return k_unit_err_none;
}

__unit_callback void unit_teardown() {}
__unit_callback void unit_reset() { s_fx.clear(); }
__unit_callback void unit_resume() {}
__unit_callback void unit_suspend() {}

__unit_callback void unit_render(const float *in, float *out, uint32_t frames) { s_fx.process(in, out, frames); }

__unit_callback void unit_set_param_value(uint8_t id, int32_t value) {
  if (id >= Radio::P_NUM) return;
  value = clipminmaxi32(unit_header.params[id].min, value, unit_header.params[id].max);
  cached_values[id] = value;
  s_fx.setParam(id, value);
}

__unit_callback int32_t unit_get_param_value(uint8_t id) {
  if (id >= UNIT_MODFX_MAX_PARAM_COUNT) return 0;
  return cached_values[id];
}

__unit_callback const char *unit_get_param_str_value(uint8_t id, int32_t value) {
  if (id >= Radio::P_NUM) return nullptr;
  value = clipminmaxi32(unit_header.params[id].min, value, unit_header.params[id].max);
  return s_fx.paramStr(id, value);
}

__unit_callback void unit_set_tempo(uint32_t tempo) { (void)tempo; }
__unit_callback void unit_tempo_4ppqn_tick(uint32_t counter) { (void)counter; }
