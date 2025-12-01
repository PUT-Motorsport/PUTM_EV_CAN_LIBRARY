/**
 * @file can_interface.cpp
 * @brief Implementation of DefaultCanInterface methods.
 */

#include "PUTM_EV_CAN_LIBRARY/include/can_interface.hpp"
#include "PUTM_EV_CAN_LIBRARY/include/message_handler.hpp"

namespace putm_ev_can {

// Implementation of the reception loop
void DefaultCanInterface::process_received_messages() {
    PUTM_CAN::CanFrame f;
    
    // Loop until RX queue is empty
    while (hal_.receive(f)) {
        // Pass received frame to the handler
        handler_.handle_message_with_default(
            f.id,
            std::span<const uint8_t>(f.data.data(), f.dlc)
        );
    }
}

} // namespace putm_ev_can