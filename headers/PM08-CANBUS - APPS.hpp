//Generated on Mon Mar 21 18:53:53 2022
#ifndef APPS
#define APPS

#include <cstdint>
#include "hal_can.hpp"
#include "message_abstraction.hpp"


enum struct Apps_states: uint8_t {
	Power_up,
	Normal_operation,
	Sensor_Implausiblity,
};

struct __attribute__ ((packed)) Apps_main{
	uint16_t pedal_position; // pedal position form 0 – 1000 where 0 means pedal not pressed
	uint8_t counter; // counts from 0-255 to ensure all frames are recived
	int8_t position_diff; // pedal position derivative in [count / second]
	Apps_states device_state; // -----------------------------------------------------------------
};

const uint16_t APPS_MAIN_CAN_ID = 0;
const uint8_t APPS_MAIN_CAN_DLC = sizeof(Apps_main);

const CAN_TxHeaderTypeDef can_tx_header_apps{
 APPS_CAN_ID, 0xFFF, CAN_ID_STD, CAN_RTR_DATA, APPS_CAN_DLC, DISABLE};

#endif

