/*
 *  RADIOLINA v0.5 - una radiolina che ogni tanto trasmette una frase cantata (ispirata a GHOST RADIO)
 *  Korg NTS-1 digital kit mkII (logue SDK v2), oscillatore
 */
#include "unit_osc.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_osc,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x15U,        // 21
    .version = 0x00000500U,  // 0.5.0
    .name = "RADIOLINA",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 512, k_unit_param_type_none, 1, 0, 0, {"TUNE"}},       // A: sintonia (stazione al centro)
        {0, 1023, 0, 358, k_unit_param_type_none, 1, 0, 0, {"EVERY"}},      // B: ogni quanto torna la frase
        {0, 100, 0, 70, k_unit_param_type_percent, 0, 0, 0, {"SIGNAL"}},
        {0, 100, 0, 60, k_unit_param_type_percent, 0, 0, 0, {"RADIO"}},
        {0, 100, 0, 40, k_unit_param_type_percent, 0, 0, 0, {"NOISE"}},
        {0, 100, 0, 20, k_unit_param_type_percent, 0, 0, 0, {"DRIFT"}},
        {0, 100, 0, 15, k_unit_param_type_percent, 0, 0, 0, {"GHOST"}},
        {0, 11, 0, 2, k_unit_param_type_strings, 0, 0, 0, {"PLAY"}},        // AUTO/TRIG/PITCH x A/B/ALT/RND
        {0, 100, 0, 70, k_unit_param_type_percent, 0, 0, 0, {"PAD"}},       // volume degli accordi
        {0, 20, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"CHORDS"}},      // OFF / MAJ MIN DOR MIX x FIX 1 2 4 8 BAR
    },
};
