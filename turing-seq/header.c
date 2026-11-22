/*
 *  File: header.c
 *  TURING SEQ v1.3 - oscillatore auto-sequenziato a registro a scorrimento
 *  per Korg NTS-1 digital kit mkII (logue SDK v2)
 */
#include "unit_osc.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_osc,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x1U,
    .version = 0x00010300U,  // 1.3.0
    .name = "TURING SEQ",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 768, k_unit_param_type_none, 0, 0, 0, {"LOCK"}},     // A: CW = loop bloccato
        {0, 1023, 0, 341, k_unit_param_type_none, 0, 0, 0, {"RANGE"}},    // B: 0..36 semitoni
        {0, 30, 0, 6, k_unit_param_type_strings, 0, 0, 0, {"LENGTH"}},    // 2..32 passi
        {0, 6, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"RATE"}},
        {0, 27, 0, 10, k_unit_param_type_strings, 0, 0, 0, {"SCALE"}},    // 28 scale
        {0, 13, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"WAVE"}},      // 14 forme d'onda
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"TIMBRE"}},  // carattere dell'onda
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"T.MOD"}},    // il registro modula il timbro
        {0, 6, 0, 3, k_unit_param_type_strings, 0, 0, 0, {"OCTAVE"}},     // -3..+3 sulla sequenza in corsa
        {0, 7, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"TRIG"}},       // reset/gate/slide combinati
    },
};
