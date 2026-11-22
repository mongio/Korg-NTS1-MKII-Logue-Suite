/*
 *  STANZE v0.2 - riverbero a quattro stanze (NEBBIA, FOSCHIA, TEATRO, CATTEDRALE) per Mario Texture.
 *  Ricostruzione "in camera pulita" del carattere di quattro riverberi logue per Prologue: codice scritto da zero,
 *  valori ricavati da misure in emulazione. Korg NTS-1 digital kit mkII (logue SDK v2), modulo revfx.
 */
#include "unit_revfx.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_revfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x13U,        // 19
    .version = 0x00000200U,  // 0.2.0
    .name = "STANZE",
    .num_params = 6,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 512, k_unit_param_type_none, 1, 0, 0, {"TIME"}},        // A: decadimento (al massimo la coda si congela)
        {0, 1023, 0, 512, k_unit_param_type_none, 1, 0, 0, {"DEPTH"}},       // B: filtro d'uscita (passa alto o passa basso)
        {-1000, 1000, 0, 0, k_unit_param_type_drywet, 1, 1, 0, {"MIX"}},     // REV + B: mix a potenza costante
        {0, 3, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"MODE"}},          // NEBBIA, FOSCHIA, TEATRO, CATTEDRALE
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"MOD"}},        // modulazione (50% = originale)
        {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"SIZE"}},       // dimensione (50% = originale)
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
    },
};
