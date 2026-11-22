/*
 *  STANZE v0.2 - interfaccia unit revfx per NTS-1 mkII
 */
#include "unit_revfx.h"
#include "utils/int_math.h"
#include "prorev.h"
constexpr float ProRev::kBase[4]; constexpr float ProRev::kRT[11];
constexpr float ProRev::kHPf[11]; constexpr float ProRev::kHPq[11]; constexpr float ProRev::kHPg[11];
constexpr float ProRev::kLPf[11]; constexpr float ProRev::kLPq[11]; constexpr float ProRev::kLPg[11];
constexpr float ProRev::kGS[11]; constexpr float ProRev::kGL[11];
constexpr float ProRev::kBassC; constexpr float ProRev::kShelfC; constexpr float ProRev::kShelfK;
static ProRev s_rev;
static int32_t s_cache[UNIT_REVFX_MAX_PARAM_COUNT];
__unit_callback int8_t unit_init(const unit_runtime_desc_t *desc) {
  if (!desc) return k_unit_err_undef;
  if (desc->target != unit_header.target) return k_unit_err_target;
  if (!UNIT_API_IS_COMPAT(desc->api)) return k_unit_err_api_version;
  if (desc->samplerate != 48000) return k_unit_err_samplerate;
  if (desc->input_channels != 2 || desc->output_channels != 2) return k_unit_err_geometry;
  if (!desc->hooks.sdram_alloc) return k_unit_err_memory;
  float *mem = (float *)desc->hooks.sdram_alloc(ProRev::kFloats * sizeof(float));   // ~257 KB
  if (!mem) return k_unit_err_memory;
  s_rev.init(mem, unit_header.params[ProRev::P_MODE].init);
  for (int id = 0; id < UNIT_REVFX_MAX_PARAM_COUNT; ++id) {
    s_cache[id] = (int32_t)unit_header.params[id].init;
    if (id < ProRev::P_NUM && id != ProRev::P_MODE) s_rev.setParam(id, s_cache[id]);
  }
  s_rev.clear();
  return k_unit_err_none;
}
__unit_callback void unit_teardown() {}
__unit_callback void unit_reset() { s_rev.clear(); }
__unit_callback void unit_resume() {}
__unit_callback void unit_suspend() {}
__unit_callback void unit_render(const float *in, float *out, uint32_t frames) { s_rev.process(in, out, frames); }
__unit_callback void unit_set_param_value(uint8_t id, int32_t value) {
  if (id >= ProRev::P_NUM) return;
  value = clipminmaxi32(unit_header.params[id].min, value, unit_header.params[id].max);
  s_cache[id] = value; s_rev.setParam(id, value);
}
__unit_callback int32_t unit_get_param_value(uint8_t id) { return id < UNIT_REVFX_MAX_PARAM_COUNT ? s_cache[id] : 0; }
__unit_callback const char *unit_get_param_str_value(uint8_t id, int32_t value) { return s_rev.paramStr(id, value); }
__unit_callback void unit_set_tempo(uint32_t) {}
__unit_callback void unit_tempo_4ppqn_tick(uint32_t) {}
