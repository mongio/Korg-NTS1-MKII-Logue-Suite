/*
 *  CAOS v0.2 - modulazione guidata dall'attrattore di Lorenz (panorama, filtro, ritardo)
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x1AU,        // 26
    .version = 0x00000200U,  // 0.2.0
    .name = "CAOS",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 512, k_unit_param_type_none, 1, 0, 0, {"SPEED"}},    // A: velocita' del sistema
        {0, 1023, 0, 767, k_unit_param_type_none, 1, 0, 0, {"CHAOS"}},    // B: da regolare a imprevedibile
        {0, 3, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"DEST"}},       // TUTTO RITARDO FILTRO PAN
        {0, 100, 0, 80, k_unit_param_type_percent, 0, 0, 0, {"DEPTH"}},
        {0, 4, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"DTYPE"}},      // FLANGE CHORUS VIBRATO RING DRIVE
        {0, 100, 0, 40, k_unit_param_type_percent, 0, 0, 0, {"FEEDBK"}},
        {0, 100, 0, 55, k_unit_param_type_percent, 0, 0, 0, {"CUTOFF"}},
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"RESO"}},
        {0, 100, 0, 90, k_unit_param_type_percent, 0, 0, 0, {"WIDTH"}},
        {0, 100, 0, 85, k_unit_param_type_percent, 0, 0, 0, {"MIX"}},
    },
};
