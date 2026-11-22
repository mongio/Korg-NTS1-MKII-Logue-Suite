/*
 *  File: header.c
 *  RICORDO-S v0.2 - variante di RICORDO con TUBO shimmer, LOWCUT e DECAY fino a 100 s: riverbero la cui coda invecchia mentre decade
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo revfx
 */
#include "unit_revfx.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_revfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0xCU,
    .version = 0x00000200U,  // 0.2.0
    .name = "RICORDO-S",
    .num_params = 11,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 520, k_unit_param_type_none, 1, 0, 0, {"DECAY"}},    // A: durata coda 0.5..100 s
        {0, 1023, 0, 450, k_unit_param_type_none, 1, 0, 0, {"AGE"}},      // B: invecchiamento per giro
        {-1000, 1000, 0, 0, k_unit_param_type_drywet, 1, 1, 0, {"MIX"}},  // come template KORG
        {0, 4, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"ERA"}},        // NASTRO/VINILE/RADIO/CERA/TUBO
        {0, 100, 0, 60, k_unit_param_type_percent, 0, 0, 0, {"SIZE"}},
        {0, 100, 0, 70, k_unit_param_type_percent, 0, 0, 0, {"TONE"}},
        {0, 100, 0, 20, k_unit_param_type_percent, 0, 0, 0, {"WOW"}},
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"LOWCUT"}},
        {0, 100, 0, 20, k_unit_param_type_percent, 0, 0, 0, {"HISS"}},
        {0, 100, 0, 0, k_unit_param_type_percent, 0, 0, 0, {"SINK"}},
        {0, 1, 0, 0, k_unit_param_type_onoff, 0, 0, 0, {"FREEZE"}},
    },
};
