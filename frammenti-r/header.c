/*
 *  File: header.c
 *  FRAMMENTI-R v0.1 - granulare / micro-loop con riverbero a piastra e shimmer, per lo slot REVERB
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo revfx
 */
#include "unit_revfx.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_revfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0xFU,
    .version = 0x00000100U,  // 0.1.0
    .name = "FRAMMENTI-R",
    .num_params = 11,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 400, k_unit_param_type_none, 1, 0, 0, {"TIME"}},      // A: dimensione grani / loop / tempo
        {0, 1023, 0, 400, k_unit_param_type_none, 1, 0, 0, {"DPTH"}},      // B: attivita' del modo
        {-1000, 1000, 0, 0, k_unit_param_type_drywet, 1, 1, 0, {"MIX"}},   // come template KORG
        {0, 3, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"MODE"}},        // CLOUD/LOOP/REVERSE/TAPS
        {0, 100, 0, 35, k_unit_param_type_percent, 0, 0, 0, {"FEEDBK"}},
        {0, 100, 0, 55, k_unit_param_type_percent, 0, 0, 0, {"SPACE"}},    // riverbero a piastra: quantita' e coda
        {0, 100, 0, 80, k_unit_param_type_percent, 0, 0, 0, {"TONE"}},
        {0, 7, 0, 3, k_unit_param_type_strings, 0, 0, 0, {"PITCH"}},
        {0, 1, 0, 0, k_unit_param_type_onoff, 0, 0, 0, {"SYNC"}},
        {0, 1, 0, 0, k_unit_param_type_onoff, 0, 0, 0, {"HOLD"}},          // congela il buffer
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"SHIMMER"}},  // ottava sopra nella coda
    },
};
