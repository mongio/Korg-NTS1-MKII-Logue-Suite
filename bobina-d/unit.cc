/*
 *  File: unit.cc
 *  BOBINA-D v0.4 - interfaccia unit delfx per NTS-1 mkII (basata sul template KORG, BSD-3)
 */
#include "unit_delfx.h"
#include "utils/int_math.h"
#include "bobina.h"

static Bobina s_fx;
static int32_t cached_values[UNIT_DELFX_MAX_PARAM_COUNT];

__unit_callback int8_t unit_init(const unit_runtime_desc_t *desc) {
  if (!desc) return k_unit_err_undef;
  if (desc->target != unit_header.target) return k_unit_err_target;
  if (!UNIT_API_IS_COMPAT(desc->api)) return k_unit_err_api_version;
  if (desc->samplerate != 48000) return k_unit_err_samplerate;
  if (desc->input_channels != 2 || desc->output_channels != 2) return k_unit_err_geometry;
  if (!desc->hooks.sdram_alloc) return k_unit_err_memory;

  // buffer stereo int16: prova 14 s, 10 s, 7 s, 5 s
  static const uint32_t secs[] = {14, 10, 7, 5};
  int16_t *buf = nullptr; uint32_t frames = 0;
  for (int k = 0; k < 4 && !buf; ++k) {
    const uint32_t f = secs[k] * 48000u;
    const uint32_t bytes = f * 2u * sizeof(int16_t);
    if (desc->hooks.sdram_avail && desc->hooks.sdram_avail() < bytes + 64) continue;
    buf = (int16_t *)desc->hooks.sdram_alloc(bytes);
    if (buf) frames = f;
  }
  if (!buf) return k_unit_err_memory;
  for (uint32_t i = 0; i < frames * 2; ++i) buf[i] = 0;

  s_fx.init(buf, frames);
  for (int id = 0; id < UNIT_DELFX_MAX_PARAM_COUNT; ++id) {
    cached_values[id] = static_cast<int32_t>(unit_header.params[id].init);
    if (id < Bobina::P_NUM) s_fx.setParam(id, cached_values[id]);
  }
  return k_unit_err_none;
}

__unit_callback void unit_teardown() {}
__unit_callback void unit_reset() {}
__unit_callback void unit_resume() {}
__unit_callback void unit_suspend() {}

__unit_callback void unit_render(const float *in, float *out, uint32_t frames) { s_fx.process(in, out, frames); }

__unit_callback void unit_set_param_value(uint8_t id, int32_t value) {
  if (id >= Bobina::P_NUM) return;
  value = clipminmaxi32(unit_header.params[id].min, value, unit_header.params[id].max);
  cached_values[id] = value;
  s_fx.setParam(id, value);
}

__unit_callback int32_t unit_get_param_value(uint8_t id) {
  if (id >= UNIT_DELFX_MAX_PARAM_COUNT) return 0;
  return cached_values[id];
}

__unit_callback const char *unit_get_param_str_value(uint8_t id, int32_t value) {
  if (id >= Bobina::P_NUM) return nullptr;
  value = clipminmaxi32(unit_header.params[id].min, value, unit_header.params[id].max);
  return s_fx.paramStr(id, value);
}

__unit_callback void unit_set_tempo(uint32_t tempo) {
  const float bpm = (tempo >> 16) + (tempo & 0xFFFF) / 65536.f;
  s_fx.setTempo(bpm);
}
__unit_callback void unit_tempo_4ppqn_tick(uint32_t counter) { (void)counter; }
