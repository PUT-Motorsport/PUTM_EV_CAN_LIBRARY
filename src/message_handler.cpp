/**
 * @file message_handler.cpp
 * @brief Implementation of message routing logic.
 */

#include "PUTM_EV_CAN_LIBRARY/include/message_handler.hpp"

namespace putm_ev_can {

void MessageHandler::handle_message_with_default(CanId id, std::span<const uint8_t> data) {
    // Validate data length
    if (data.size() > 8) return;
    
    // Enter critical section
    ScopedLock guard(*this);
    
    bool handled = false;

    // Linear scan through the static callback array
    // This is efficient for small numbers of callbacks (e.g. < 50)
    for (const auto& entry : callbacks_) {
        if (entry.active && entry.id == id) {
            // Found a match - invoke callback
            if (entry.callback) {
                entry.callback(data);
            }
            handled = true;
            break; // Stop searching
        }
    }

    // If no specific callback was found, try the default one
    if (!handled && default_callback_) {
        default_callback_(id, data);
    }
}

} // namespace putm_ev_can