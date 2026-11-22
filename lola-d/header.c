#include "unit_delfx.h"
// LOLA-D v0.1 - delay con abbai di cane nelle ripetizioni (motore di SEAGULL).
// Abbai: BigSoundBank n. 0112 (Joseph Sardin) e OpenGameArt "Dog barking mono" (Brandon Morris), entrambi CC0.
const __unit_header unit_header_t unit_header = {
 .header_size=sizeof(unit_header_t), .target=UNIT_TARGET_PLATFORM|k_unit_module_delfx,
 .api=UNIT_API_VERSION, .dev_id=0x4D545854U, .unit_id=0x14U,
 .version=0x00000100U, .name="LOLA-D", .num_params=10,
 .params={
 // min, max, center, default, type, frac, frac_mode, reserved, name
 {0,1023,0,460,k_unit_param_type_none,1,0,0,{"TIME"}},           // knob A
 {0,1023,0,440,k_unit_param_type_none,1,0,0,{"FEEDBACK"}},       // knob B
 {-1000,1000,0,-100,k_unit_param_type_drywet,1,1,0,{"MIX"}},     // DEL + knob B (template KORG)
 {0,8,0,8,k_unit_param_type_strings,0,0,0,{"BARK"}},             // BAU1-BAU8 / RANDOM
 {0,1000,0,500,k_unit_param_type_none,0,0,0,{"INJECT"}},
 {0,1000,0,500,k_unit_param_type_none,0,0,0,{"CHANCE"}},
 {0,1000,0,500,k_unit_param_type_none,0,0,0,{"PITCH"}},
 {0,1000,0,150,k_unit_param_type_none,0,0,0,{"DEGRADE"}},
 {0,1000,0,500,k_unit_param_type_none,0,0,0,{"DISTANCE"}},
 {0,1000,0,600,k_unit_param_type_none,0,0,0,{"SPREAD"}}
 }};
