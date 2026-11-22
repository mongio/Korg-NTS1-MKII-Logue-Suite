/*
 *  SUPERFLANGER v0.1 - flanger stereo CLASSIC / TAPE (attraverso lo zero) / BARBERPOLE
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x10U,        // 16
    .version = 0x00000100U,  // 0.1.0
    .name = "SUPERFLANGER",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 300, k_unit_param_type_none, 1, 0, 0, {"RATE"}},     // A: velocita' 0,02..10 Hz
        {0, 1023, 0, 650, k_unit_param_type_none, 1, 0, 0, {"DEPTH"}},    // B: escursione
        {0, 3, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"MODE"}},       // CLASSIC/TAPE/BARBUP/BARBDN
        {0, 100, 0, 55, k_unit_param_type_percent, 0, 0, 0, {"FEEDBK"}},
        {0, 1, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"COLOR"}},      // POS/NEG (polarita' del feedback)
        {0, 100, 0, 25, k_unit_param_type_percent, 0, 0, 0, {"MANUAL"}},  // ritardo di partenza
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"STEREO"}},  // sfasamento L/R
        {0, 3, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"SHAPE"}},      // SINE/TRI/RANDOM/ENV
        {0, 100, 0, 20, k_unit_param_type_percent, 0, 0, 0, {"AGE"}},     // grana BBD: scuro e saturo
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"MIX"}},     // 50% = flanging classico
    },
};
