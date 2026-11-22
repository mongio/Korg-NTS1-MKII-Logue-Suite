/*
 *  File: header.c
 *  BOCCA v0.2 - filtro a formanti che fa parlare qualsiasi suono
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x9U,
    .version = 0x00000200U,  // 0.2.0
    .name = "BOCCA",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 512, k_unit_param_type_none, 1, 0, 0, {"VOWEL"}},   // A: U-O-A-E-I
        {0, 1023, 0, 600, k_unit_param_type_none, 1, 0, 0, {"DEPTH"}},   // B: quanto si muove la bocca
        {0, 5, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"MOD"}},       // LFO/RANDOM/ENV/TURING/LOOP/CHAT
        {0, 60, 0, 30, k_unit_param_type_strings, 0, 0, 0, {"RATE"}},    // 0-49 libero (0,05-12 Hz), 50-60 a tempo
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"VOICE"}},  // maschile .. bambino
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"RESO"}},
        {0, 100, 0, 15, k_unit_param_type_percent, 0, 0, 0, {"GLIDE"}},
        {0, 100, 0, 20, k_unit_param_type_percent, 0, 0, 0, {"DRIVE"}},
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"BODY"}},   // quanto suono originale resta sotto
        {0, 100, 0, 100, k_unit_param_type_percent, 0, 0, 0, {"MIX"}},
    },
};
