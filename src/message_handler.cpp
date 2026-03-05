#include "message_handler.hpp"

namespace putm_ev_can {

void MessageHandler::handle_message(PUTM_CAN::CanId id, std::span<const uint8_t> data) {
    LockGuard lock(protection_); 

    for (const auto& entry : callbacks_) {
        if (entry.active && entry.id == id) {
            if (entry.callback) {
                entry.callback(data);
            }
            return;
        }
    }
}

} // namespace putm_ev_can