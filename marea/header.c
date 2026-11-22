/*
 *  MAREA v0.2 - modulazione a onde che fa ruotare le stazioni: FLANGE CHORUS RISACCA SALE RING ROVINA ECO
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x19U,        // 25
    .version = 0x00000200U,  // 0.2.0
    .name = "MAREA",
    .num_params = 8,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 512, k_unit_param_type_none, 1, 0, 0, {"MARE"}},      // A: da bonaccia a burrasca
        {0, 1023, 0, 512, k_unit_param_type_none, 1, 0, 0, {"PERIODO"}},   // B: durata delle onde 0,5..16 s
        {0, 3, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"ROTA"}},        // ONDA MAREA CASO FISSO
        {0, 2, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"STAZIONI"}},    // TUTTE PULITE SPORCHE
        {0, 6, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"STAZ"}},        // stazione di partenza / fissa
        {0, 100, 0, 80, k_unit_param_type_percent, 0, 0, 0, {"AMOUNT"}},
        {0, 100, 0, 70, k_unit_param_type_percent, 0, 0, 0, {"STEREO"}},
        {0, 100, 0, 100, k_unit_param_type_percent, 0, 0, 0, {"MIX"}},
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
    },
};
