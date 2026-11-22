/*
 *  ARMONIA v0.1 - armonizzatore denso: due voci trasposte raddoppiate, scala, shimmer, grana, reverse, deriva, stereo
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x1BU,        // 27
    .version = 0x00000100U,  // 0.1.0
    .name = "ARMONIA",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 48, 24, 31, k_unit_param_type_none, 0, 0, 0, {"INTERV"}},     // A: 24 = altezza originale, 31 = quinta sopra
        {0, 1023, 0, 614, k_unit_param_type_none, 1, 0, 0, {"MIX"}},      // B: originale / voci
        {0, 49, 0, 13, k_unit_param_type_strings, 0, 0, 0, {"VOICE2"}},   // OFF, -24..24 (iniziale -12)
        {0, 4, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"SCALE"}},      // CHROM MAJOR MINOR PENTA OCTAVE
        {0, 100, 0, 45, k_unit_param_type_percent, 0, 0, 0, {"DETUNE"}},
        {0, 100, 0, 25, k_unit_param_type_percent, 0, 0, 0, {"SHIMMER"}},
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"GRANA"}},
        {0, 2, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"REVERSE"}},    // OFF ON MISTO
        {0, 100, 0, 25, k_unit_param_type_percent, 0, 0, 0, {"DRIFT"}},
        {0, 100, 0, 80, k_unit_param_type_percent, 0, 0, 0, {"STEREO"}},
    },
};
