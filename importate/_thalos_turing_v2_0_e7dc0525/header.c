#include "unit_osc.h"
const __unit_header unit_header_t unit_header = {
 .header_size=sizeof(unit_header_t), .target=UNIT_TARGET_PLATFORM|k_unit_module_osc, .api=UNIT_API_VERSION,
 .dev_id=0x4D545854U, .unit_id=0x54555233U, .version=0x00020000U, .name="THALOS T V2",
 .num_params=10,
 .params={
  {0,100,0,50,k_unit_param_type_none,0,0,0,{"Alt"}},
  {0,100,0,0,k_unit_param_type_none,0,0,0,{"Shape"}},
  {0,100,0,18,k_unit_param_type_none,0,0,0,{"Slop"}},
  {0,19,0,0,k_unit_param_type_strings,0,0,0,{"Algo"}},
  {0,100,0,55,k_unit_param_type_none,0,0,0,{"Amount"}},
  {0,100,0,35,k_unit_param_type_none,0,0,0,{"Freq"}},
  {0,3,0,0,k_unit_param_type_strings,0,0,0,{"Mod"}},
  {0,10,0,0,k_unit_param_type_strings,0,0,0,{"Scale"}},
  {0,14,0,0,k_unit_param_type_strings,0,0,0,{"Edit"}},
  {0,100,0,0,k_unit_param_type_strings,0,0,0,{"Value"}},
 }};
