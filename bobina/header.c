/*
 *  File: header.c
 *  BOBINA v0.4 - looper a nastro con varispeed e usura progressiva
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo revfx (ultimo della catena)
 */
#include "unit_revfx.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_revfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x6U,
    .version = 0x00000400U,  // 0.4.0
    .name = "BOBINA",
    .num_params = 11,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 3, 0, 1, k_unit_param_type_none, 0, 0, 0, {"ACTION"}},     // A: 0 CLEAR (tenuto) / 1 STOP / 2 PLAY / 3 REC-DUB
        {0, 1023, 0, 768, k_unit_param_type_none, 1, 0, 0, {"SPEED"}},    // B: velocita' -2x..+2x (768 = 1x)
        {-1000, 1000, 0, 0, k_unit_param_type_drywet, 1, 1, 0, {"MIX"}},  // come template KORG
        {0, 6, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"SYNC"}},       // OFF/BEAT/BAR/AUTO1/AUTO2/AUTO4/AUTO8
        {0, 100, 0, 90, k_unit_param_type_percent, 0, 0, 0, {"DECAY"}},   // quanto resta a ogni overdub
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"WEAR"}},     // usura del nastro a ogni giro
        {0, 100, 0, 15, k_unit_param_type_percent, 0, 0, 0, {"WOW"}},
        {0, 100, 0, 100, k_unit_param_type_percent, 0, 0, 0, {"TONE"}},
        {0, 1, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"SPDMODE"}},    // FREE/STEP
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"INERTIA"}}, // inerzia del nastro
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"FADE"}},     // il loop si spegne a ogni giro
    },
};
