/*
 *  File: header.c
 *  ROMANZA v0.3 - oscillatore vocale a frammenti di parole (PSOLA, banco a 8 kHz) per Mario Texture
 *  Korg NTS-1 digital kit mkII (logue SDK v2)
 */
#include "unit_osc.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_osc,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0xDU,
    .version = 0x00000300U,  // 0.3.0
    .name = "ROMANZA",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 0, k_unit_param_type_none, 0, 0, 0, {"SCAN"}},       // A: posizione nel banco vocale
        {0, 1023, 0, 815, k_unit_param_type_none, 0, 0, 0, {"SPEED"}},    // B: 0 = freeze ... 2x
        {0, 3, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"MODE"}},       // FRAG/SEQ/RAND/BABBLE
        {0, 2, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"PLAY"}},       // ONESHOT/LOOP/BOUNCE
        {0, 1, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"PITCH"}},      // KEY/MELODY
        {0, 24, 0, 12, k_unit_param_type_strings, 0, 0, 0, {"FORMANT"}},  // -12..+12
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"JITTER"}},
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"VIBRATO"}},
        {0, 11, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"VOICES"}},     // MONO/CHOIR/HOLD2-4/accordi
        {0, 100, 0, 10, k_unit_param_type_percent, 0, 0, 0, {"TAPE"}},
    },
};
