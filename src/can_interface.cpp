/**
 * @file can_interface.cpp
 * @brief Implementation of high-level CAN interface using DBC-generated code.
 */

#include "can_interface.hpp"
#include "can_hal.hpp"
#include "generated/putm_can_1.h"  // Generated from PUTM_CAN_1.dbc

namespace putm_can {

/**
 * @brief Constructor initializes HAL.
 * @param hal_ptr Pointer to hardware abstraction layer
 */
CanInterface::CanInterface(ICanHal* hal_ptr) : hal(hal_ptr) {
    if (hal) hal->init();
}

/**
 * @brief Handles all incoming CAN frames.
 *
 * Unpacks using cantools-generated functions and updates internal state.
 */
void CanInterface::handle_incoming_messages() {
    CanFrame frame;
    while (hal->receive(frame)) {
        switch (frame.id) {
            case PUTM_CAN_1_BMS_HV_MAIN_FRAME_ID: {
                bms_hv_main_t msg{};
                if (bms_hv_main_unpack(&msg, frame.data.data(), frame.dlc) == 0) {
                    float voltage = bms_hv_main_voltage_sum_decode(msg.voltage_sum);
                    // Process voltage...
                }
                break;
            }
            // Add other message handlers here
            default:
                break;
        }
    }
}

/**
 * @brief Sends BMS HV main message.
 * @param msg Pre-filled DBC structure
 * @return true if sent successfully
 */
bool CanInterface::send_bms_hv_main(const bms_hv_main_t& msg) {
    return send(PUTM_CAN_1_BMS_HV_MAIN_FRAME_ID, msg);
}

} // namespace putm_can