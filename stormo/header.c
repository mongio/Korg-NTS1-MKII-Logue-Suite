/*
 *  STORMO v0.1 - chorus a stormo: copie del suono che volano come uno stormo di storni
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x16U,        // 22
    .version = 0x00000100U,  // 0.1.0
    .name = "STORMO",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 358, k_unit_param_type_none, 1, 0, 0, {"SPEED"}},     // A: velocita' del volo
        {0, 1023, 0, 512, k_unit_param_type_none, 1, 0, 0, {"DEPTH"}},     // B: profondita' dello stormo (ritardi)
        {0, 6, 0, 4, k_unit_param_type_strings, 0, 0, 0, {"BIRDS"}},       // 2..8 uccelli (sul display)
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"COHESION"}},
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"PREDATOR"}},
        {0, 100, 0, 80, k_unit_param_type_percent, 0, 0, 0, {"WIDTH"}},
        {0, 2, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"SYNC"}},        // OFF / BEAT / BAR
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"TONE"}},
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"FEEDBK"}},
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"MIX"}},
    },
};
