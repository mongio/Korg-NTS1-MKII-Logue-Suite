/*
 *  TELAIO v0.1 - la macchina di Turing applicata alla modulazione
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x18U,        // 24
    .version = 0x00000100U,  // 0.1.0
    .name = "TELAIO",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 614, k_unit_param_type_none, 1, 0, 0, {"AMOUNT"}},   // A: quanto agisce
        {0, 1023, 0, 767, k_unit_param_type_none, 1, 0, 0, {"LOCK"}},     // B: destra = si ripete, sinistra = muta sempre
        {0, 4, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"DEST"}},       // FILTER PAN PITCH VOLUME FILTPAN
        {0, 5, 0, 3, k_unit_param_type_strings, 0, 0, 0, {"RATE"}},       // 1BAR 2BEAT 1BEAT 8TH 16TH 32ND
        {0, 14, 0, 6, k_unit_param_type_strings, 0, 0, 0, {"LENGTH"}},    // 2..16 passi
        {0, 100, 0, 20, k_unit_param_type_percent, 0, 0, 0, {"SLIDE"}},
        {0, 4, 0, 2, k_unit_param_type_strings, 0, 0, 0, {"SCALE"}},      // CHROM MAJOR MINOR PENTA OCTAVE
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"OFFSET"}},
        {0, 100, 0, 40, k_unit_param_type_percent, 0, 0, 0, {"RESO"}},
        {0, 100, 0, 100, k_unit_param_type_percent, 0, 0, 0, {"MIX"}},
    },
};
