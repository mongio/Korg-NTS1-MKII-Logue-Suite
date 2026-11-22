/*
 *  SUPERORBITAL v0.2 - estensione di ORBITAL14 (pan spettrale a 4 bande + 2 campane EQ animate)
 *  con i modi DOPPLER (4 satelliti in orbita) e ROTARY (altoparlante rotante).
 *  Korg NTS-1 digital kit mkII (logue SDK v2), modulo modfx
 */
#include "unit_modfx.h"
const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_modfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x4D545854U,   // 'MTXT'
    .unit_id = 0x12U,        // 18
    .version = 0x00000200U,  // 0.2.0
    .name = "SUPERORBITAL",
    .num_params = 10,
    .params = {
        // min, max, center, default, type, frac, frac_mode, reserved, name
        {0, 1023, 0, 409, k_unit_param_type_none, 1, 0, 0, {"SPEED"}},     // A: 0,012..32 Hz (ROTARY: <50% lento, >=50% veloce)
        {0, 1023, 0, 921, k_unit_param_type_none, 1, 0, 0, {"SPREAD"}},    // B: ampiezza stereo
        {0, 10, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"MODE"}},       // ... DOPPLER ROTARY XPHASER XFLANGE XJET
        {0, 1000, 0, 850, k_unit_param_type_none, 0, 0, 0, {"EQ AMT"}},
        {0, 1000, 0, 700, k_unit_param_type_none, 0, 0, 0, {"MOTION"}},
        {0, 1000, 0, 450, k_unit_param_type_none, 0, 0, 0, {"DEPTH"}},     // DOPPLER/ROTARY: quantita' di Doppler
        {0, 1000, 0, 800, k_unit_param_type_none, 0, 0, 0, {"SWEEP"}},
        {0, 1000, 0, 650, k_unit_param_type_none, 0, 0, 0, {"Q"}},
        {0, 1000, 0, 850, k_unit_param_type_none, 0, 0, 0, {"PEAK EQ"}},
        {0, 1000, 0, 1000, k_unit_param_type_none, 0, 0, 0, {"MIX"}},
    },
};
