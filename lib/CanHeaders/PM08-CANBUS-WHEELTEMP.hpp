// Generated on Wed Jun 15 10:27:25 2022
#ifndef WHEELTEMP
#define WHEELTEMP

#include <cstdint>

namespace PUTM_CAN {

struct __attribute__((packed)) WheelTemp_main {
  uint8_t wheelTemp[8];
};

const uint16_t WHEELTEMP_MAIN_CAN_ID = 0x69;
const uint8_t WHEELTEMP_MAIN_CAN_DLC = sizeof(WheelTemp_main);
const uint8_t WHEELTEMP_MAIN_FREQUENCY = 1;

#ifndef PUTM_USE_CAN_FD

const CAN_TxHeaderTypeDef can_tx_header_WHEELTEMP_MAIN{
    WHEELTEMP_MAIN_CAN_ID,  0xFFF,  CAN_ID_STD, CAN_RTR_DATA,
    WHEELTEMP_MAIN_CAN_DLC, DISABLE};

#endif

}   // namespace PUTM_CAN

#endif
