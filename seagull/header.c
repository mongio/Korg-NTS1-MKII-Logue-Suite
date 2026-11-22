#include "unit_delfx.h"
// SEAGULL v0.1 - delay con versi di gabbiano lontani nelle ripetizioni (da VOX INJECT v1.1).
// Versi: xeno-canto XC584347, Stanislas Wroza, CC BY-NC-SA 4.0.
const __unit_header unit_header_t unit_header = {
 .header_size=sizeof(unit_header_t), .target=UNIT_TARGET_PLATFORM|k_unit_module_delfx,
 .api=UNIT_API_VERSION, .dev_id=0x4D545854U, .unit_id=0xEU,
 .version=0x00000100U, .name="SEAGULL", .num_params=10,
 .params={
 // min, max, center, default, type, frac, frac_mode, reserved, name
 {0,1023,0,460,k_unit_param_type_none,1,0,0,{"TIME"}},           // knob A
 {0,1023,0,440,k_unit_param_type_none,1,0,0,{"FEEDBACK"}},       // knob B
 {-1000,1000,0,-100,k_unit_param_type_drywet,1,1,0,{"MIX"}},     // DEL + knob B (template KORG)
 {0,6,0,6,k_unit_param_type_strings,0,0,0,{"CALL"}},             // ADULT1-3 / YOUNG1-3 / RANDOM
 {0,1000,0,500,k_unit_param_type_none,0,0,0,{"INJECT"}},
 {0,1000,0,500,k_unit_param_type_none,0,0,0,{"CHANCE"}},
 {0,1000,0,500,k_unit_param_type_none,0,0,0,{"PITCH"}},
 {0,1000,0,150,k_unit_param_type_none,0,0,0,{"DEGRADE"}},
 {0,1000,0,600,k_unit_param_type_none,0,0,0,{"DISTANCE"}},
 {0,1000,0,700,k_unit_param_type_none,0,0,0,{"SPREAD"}}
 }};
