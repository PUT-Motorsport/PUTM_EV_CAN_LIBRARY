/**
 * @file can_interface.cpp
 * @brief Implementation of DefaultCanInterface methods.
 */

#include "PUTM_EV_CAN_LIBRARY/include/can_interface.hpp"
#include "PUTM_EV_CAN_LIBRARY/include/message_handler.hpp"

namespace putm_ev_can {

// Implementation of the reception loop
void DefaultCanInterface::process_received_messages() {
    // In Interrupt-Driven mode, this method is empty regarding RX.
    // Messages are pushed directly from HAL ISR -> Handler.
}

bool DefaultCanInterface::init() {
    // 1. Register the RX Callback (Lambda)
    // This lambda will be executed inside the ISR context!
    hal_.set_rx_callback([this](const PUTM_CAN::CanFrame& frame) {
        handler_.handle_message_with_default(
            frame.id,
            std::span<const uint8_t>(frame.data.data(), frame.dlc)
        );
    });

    // 2. Initialize Hardware (Enables Interrupts)
    return hal_.init();
}

} // namespace putm_ev_can