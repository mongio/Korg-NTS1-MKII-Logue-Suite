/*
 *  SUPERPHASER v0.1 - phaser vintage da 4 a 12 stadi con ensemble "string machine"
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x11U,        // 17
    .version = 0x00000100U,  // 0.1.0
    .name = "SUPERPHASER",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 250, k_unit_param_type_none, 1, 0, 0, {"RATE"}},       // A: velocita' 0,02..6 Hz
        {0, 1023, 0, 720, k_unit_param_type_none, 1, 0, 0, {"DEPTH"}},      // B: ampiezza dello sweep
        {0, 4, 0, 2, k_unit_param_type_strings, 0, 0, 0, {"STAGES"}},       // 4/6/8/10/12
        {0, 100, 0, 45, k_unit_param_type_percent, 0, 0, 0, {"COLOR"}},     // risonanza
        {0, 100, 0, 45, k_unit_param_type_percent, 0, 0, 0, {"CENTER"}},    // centro dello sweep
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"ENSEMBLE"}},  // coro "string machine"
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"STEREO"}},
        {0, 2, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"SHAPE"}},        // SINE/TRI/RANDOM
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"WARMTH"}},
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"MIX"}},       // 50% = phaser classico, 100% = vibrato
    },
};
