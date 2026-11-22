/*
 *  File: header.c
 *  LOLA v0.2 - oscillatore che abbaia (dedicato a Lola, bassotta a pelo duro)
 *  Korg NTS-1 digital kit mkII (logue SDK v2)
 */
#include "unit_osc.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_osc,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0xAU,
    .version = 0x00000200U,  // 0.2.0
    .name = "LOLA",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 250, k_unit_param_type_none, 0, 0, 0, {"SIZE"}},     // A: taglia (25% = bassotta)
        {0, 1023, 0, 400, k_unit_param_type_none, 0, 0, 0, {"ROUGH"}},    // B: ruvidita' della voce
        {0, 6, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"CALL"}},       // BARK/YIP/ARF/GROWL/WHINE/HOWL/MIX
        {0, 50, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"REPEAT"}},    // OFF, 1-39 libero, 40-50 a tempo
        {0, 1, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"PITCH"}},      // KEY/NATURAL
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"LENGTH"}},
        {0, 100, 0, 40, k_unit_param_type_percent, 0, 0, 0, {"VARY"}},
        {0, 100, 0, 60, k_unit_param_type_percent, 0, 0, 0, {"TONE"}},
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"PANT"}},     // fiatone tra un verso e l'altro
        {0, 100, 0, 30, k_unit_param_type_percent, 0, 0, 0, {"CRACK"}},   // stecche da cucciolo
    },
};
