#include "unit_delfx.h"
// VOX INJECT v1.1: buffer del delay in SDRAM, voci senza silenzio (4 voci), MIX al terzo posto come da template KORG.
const __unit_header unit_header_t unit_header = {
 .header_size=sizeof(unit_header_t), .target=UNIT_TARGET_PLATFORM|k_unit_module_delfx,
 .api=UNIT_API_VERSION, .dev_id=0x4D545854U, .unit_id=0x56584931U,
 .version=0x00010100U, .name="VOX INJECT", .num_params=8,
 .params={
 // min, max, center, default, type, frac, frac_mode, reserved, name
 {0,1023,0,460,k_unit_param_type_none,1,0,0,{"TIME"}},          // knob A
 {0,1023,0,440,k_unit_param_type_none,1,0,0,{"FEEDBACK"}},      // knob B
 {-1000,1000,0,-100,k_unit_param_type_drywet,1,1,0,{"MIX"}},    // DEL + knob B (come template KORG); -100 = 45% wet
 {0,3,0,0,k_unit_param_type_none,0,0,0,{"VOICE"}},
 {0,1000,0,500,k_unit_param_type_none,0,0,0,{"INJECT"}},
 {0,1000,0,500,k_unit_param_type_none,0,0,0,{"CHANCE"}},
 {0,1000,0,500,k_unit_param_type_none,0,0,0,{"PITCH"}},
 {0,1000,0,150,k_unit_param_type_none,0,0,0,{"DEGRADE"}}
 }};
