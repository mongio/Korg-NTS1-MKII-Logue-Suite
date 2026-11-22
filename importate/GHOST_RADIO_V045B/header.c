#include "unit_osc.h"
const __unit_header unit_header_t unit_header = {
 .header_size=sizeof(unit_header_t), .target=UNIT_TARGET_PLATFORM|k_unit_module_osc, .api=UNIT_API_VERSION,
 .dev_id=0x4D545854U, .unit_id=0x47523442U, .version=0x00040500U, .name="GhostRadio45B",
 .num_params=7,
 .params={
  {0,100,0,40,k_unit_param_type_none,0,0,0,{"Tune"}},
  {0,100,0,20,k_unit_param_type_none,0,0,0,{"Reality"}},
  {0,100,0,65,k_unit_param_type_none,0,0,0,{"Signal"}},
  {0,100,0,40,k_unit_param_type_none,0,0,0,{"Drift"}},
  {0,100,0,40,k_unit_param_type_none,0,0,0,{"Ghost"}},
  {0,100,0,0,k_unit_param_type_none,0,0,0,{"Scan"}},
  {0,3,0,1,k_unit_param_type_none,0,0,0,{"KeyMode"}},
  {0,0,0,0,k_unit_param_type_none,0,0,0,{""}}, {0,0,0,0,k_unit_param_type_none,0,0,0,{""}},
  {0,0,0,0,k_unit_param_type_none,0,0,0,{""}}, {0,0,0,0,k_unit_param_type_none,0,0,0,{""}}
 }};
