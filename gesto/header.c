/*
 *  GESTO v0.1 - registra il movimento del knob B e lo ripete a tempo come modulazione
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x17U,        // 23
    .version = 0x00000100U,  // 0.1.0
    .name = "GESTO",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 716, k_unit_param_type_none, 1, 0, 0, {"AMOUNT"}},   // A: quanto agisce la curva
        {0, 1023, 0, 512, k_unit_param_type_none, 1, 0, 0, {"GESTO"}},    // B: il knob da muovere (registrato in REC)
        {0, 4, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"DEST"}},       // FILTER PAN PITCH VOLUME FILTPAN
        {0, 2, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"ACTION"}},     // LIVE REC PLAY
        {0, 3, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"LENGTH"}},     // 1BAR 2BAR 4BAR 8BAR
        {0, 2, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"DIR"}},        // FWD REV PINGPONG
        {0, 4, 0, 2, k_unit_param_type_strings, 0, 0, 0, {"SPEED"}},      // 0.25X .. 4X
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"SMOOTH"}},
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"WEAR"}},
        {0, 100, 0, 40, k_unit_param_type_percent, 0, 0, 0, {"RESO"}},
    },
};
