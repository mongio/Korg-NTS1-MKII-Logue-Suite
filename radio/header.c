/*
 *  File: header.c
 *  RADIO v0.2 - radio a valvole anni '60 con manopola di sintonia
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x5U,
    .version = 0x00000200U,  // 0.2.0
    .name = "RADIO",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 512, k_unit_param_type_none, 1, 0, 0, {"TUNE"}},     // A: sintonia (centro = stazione)
        {0, 1023, 0, 400, k_unit_param_type_none, 1, 0, 0, {"AGE"}},      // B: eta' della radio
        {0, 3, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"BAND"}},       // FM/AM/SW/POCKET
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"FADING"}},
        {0, 100, 0, 20, k_unit_param_type_percent, 0, 0, 0, {"DRIFT"}},
        {0, 100, 0, 40, k_unit_param_type_percent, 0, 0, 0, {"STATIC"}},
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"GHOST"}},
        {0, 100, 0, 15, k_unit_param_type_percent, 0, 0, 0, {"HUM"}},
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"SEEK"}},
        {0, 100, 0, 100, k_unit_param_type_percent, 0, 0, 0, {"MIX"}},
    },
};
