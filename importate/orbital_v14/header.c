#include "unit_modfx.h"
const __unit_header unit_header_t unit_header = {
 .header_size=sizeof(unit_header_t), .target=UNIT_TARGET_PLATFORM|k_unit_module_modfx,
 .api=UNIT_API_VERSION, .dev_id=0x4D545854U, .unit_id=0x4F523134U,
 .version=0x00010400U, .name="ORBITAL14", .num_params=10,
 .params={
 {0,5,0,1,k_unit_param_type_strings,0,0,0,{"MODE"}},
 {0,1000,0,400,k_unit_param_type_none,0,0,0,{"SPEED"}},
 {0,1000,0,900,k_unit_param_type_none,0,0,0,{"SPREAD"}},
 {0,1000,0,850,k_unit_param_type_none,0,0,0,{"EQ AMT"}},
 {0,1000,0,700,k_unit_param_type_none,0,0,0,{"MOTION"}},
 {0,1000,0,1000,k_unit_param_type_none,0,0,0,{"MIX"}},
 {0,1000,0,450,k_unit_param_type_none,0,0,0,{"DEPTH"}},
 {0,1000,0,800,k_unit_param_type_none,0,0,0,{"SWEEP"}},
 {0,1000,0,650,k_unit_param_type_none,0,0,0,{"Q"}},
 {0,1000,0,850,k_unit_param_type_none,0,0,0,{"PEAK EQ"}}
 }};
