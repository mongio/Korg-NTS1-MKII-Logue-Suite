/*
 *  File: header.c
 *  FRAMMENTI v0.2 - delay granulare / micro-loop con diffusione riverberante
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo delfx
 */
#include "unit_delfx.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_delfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x3U,
    .version = 0x00000200U,  // 0.2.0
    .name = "FRAMMENTI",
    .num_params = 11,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 400, k_unit_param_type_none, 1, 0, 0, {"TIME"}},      // A: dimensione grani / loop / tempo
        {0, 1023, 0, 400, k_unit_param_type_none, 1, 0, 0, {"DPTH"}},      // B: attivita' del modo
        {-1000, 1000, 0, 0, k_unit_param_type_drywet, 1, 1, 0, {"MIX"}},   // come template KORG
        {0, 3, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"MODE"}},        // CLOUD/LOOP/REVERSE/TAPS
        {0, 100, 0, 35, k_unit_param_type_percent, 0, 0, 0, {"FEEDBK"}},
        {0, 100, 0, 40, k_unit_param_type_percent, 0, 0, 0, {"SPACE"}},    // diffusione riverberante
        {0, 100, 0, 80, k_unit_param_type_percent, 0, 0, 0, {"TONE"}},
        {0, 7, 0, 3, k_unit_param_type_strings, 0, 0, 0, {"PITCH"}},
        {0, 1, 0, 0, k_unit_param_type_onoff, 0, 0, 0, {"SYNC"}},
        {0, 1, 0, 0, k_unit_param_type_onoff, 0, 0, 0, {"HOLD"}},          // congela il buffer
        {0, 100, 0, 60, k_unit_param_type_percent, 0, 0, 0, {"SPREAD"}},   // ampiezza stereo
    },
};
