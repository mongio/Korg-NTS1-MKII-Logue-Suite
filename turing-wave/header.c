/*
 *  File: header.c
 *  TURING WAVE v1.1 - registro a scorrimento a frequenza audio:
 *  i bit del registro sono la forma d'onda.
 *  Korg NTS-1 digital kit mkII (logue SDK v2)
 */
#include "unit_osc.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_osc,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x2U,
    .version = 0x00010100U,
    .name = "TURING WAVE",
    .num_params = 9,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 920, k_unit_param_type_none, 0, 0, 0, {"LOCK"}},    // A: dx bloccato, centro rumore, sx ottava sotto
        {0, 1023, 0, 720, k_unit_param_type_none, 0, 0, 0, {"TONE"}},    // B: filtro passa-basso a inseguimento
        {0, 30, 0, 14, k_unit_param_type_strings, 0, 0, 0, {"LENGTH"}},  // 2..32 passi per ciclo
        {0, 3, 0, 2, k_unit_param_type_strings, 0, 0, 0, {"READ"}},      // 1BIT/4BIT/8BIT/SMOOTH
        {0, 1, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"MODE"}},      // TURING/LFSR
        {0, 1, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"NOTE"}},      // KEEP/NEW
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"BURST"}},   // mutazione a ogni nota
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"SUB"}},     // sub-oscillatore -1 ott
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"DRIVE"}},   // saturazione
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
    },
};
