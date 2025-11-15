/**
 * @file message_handler.cpp
 * @brief Message routing + callback management with thread safety.
 */

#include "message_handler.hpp"


namespace putm_ev_can {

static const char* message_id_to_string(CanId id) {
    // opcjonalne mapowanie do logów (przykładowe)
    switch (id) {
        case PUTM_CAN_1_PC_MAIN_DATA_FRAME_ID:        return "PC_MainData";
        case PUTM_CAN_1_PC_TEMPERATURE_DATA_FRAME_ID: return "PC_TemperatureData";
        case PUTM_CAN_1_DRIVER_INPUT_FRAME_ID:        return "DriverInput";
        case PUTM_CAN_1_PDU_DATA_FRAME_ID:            return "PDU_Data";
        default: return "Unknown";
    }
}

void MessageHandler::handle_message(CanId id, std::span<const uint8_t> data) {
    if (data.empty() || data.size() > 8) return;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = callbacks_.find(id);
    if (it != callbacks_.end()) {
        try {
            it->second(data);
        } catch (const std::exception& e) {
            std::cerr << "Callback error for 0x" << std::hex << id
                      << " (" << message_id_to_string(id) << "): " << e.what() << std::endl;
        }
    }
}

void MessageHandler::handle_message_with_default(CanId id, std::span<const uint8_t> data) {
    if (data.empty() || data.size() > 8) return;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = callbacks_.find(id);
    if (it != callbacks_.end()) {
        try {
            it->second(data);
        } catch (const std::exception& e) {
            std::cerr << "Callback error for 0x" << std::hex << id << ": " << e.what() << std::endl;
        }
    } else if (default_callback_) {
        try {
            default_callback_(id, data);
        } catch (const std::exception& e) {
            std::cerr << "Default callback error for 0x" << std::hex << id << ": " << e.what() << std::endl;
        }
    }
}

void MessageHandler::handle_messages_batch(
    const std::vector<std::pair<CanId, std::span<const uint8_t>>>& messages) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [id, data] : messages) {
        if (data.size() > 8 || data.empty()) continue;
        auto it = callbacks_.find(id);
        if (it != callbacks_.end()) {
            try {
                it->second(data);
            } catch (const std::exception& e) {
                std::cerr << "Batch callback error for 0x" << std::hex << id
                          << ": " << e.what() << std::endl;
            }
        } else if (default_callback_) {
            try {
                default_callback_(id, data);
            } catch (const std::exception& e) {
                std::cerr << "Default batch callback error for 0x" << std::hex << id
                          << ": " << e.what() << std::endl;
            }
        }
    }
}

bool MessageHandler::has_callback(CanId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return callbacks_.find(id) != callbacks_.end();
}

size_t MessageHandler::callback_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return callbacks_.size();
}

void MessageHandler::clear_callbacks() {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.clear();
}

bool MessageHandler::remove_callback(CanId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return callbacks_.erase(id) > 0;
}

std::vector<CanId> MessageHandler::get_registered_ids() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<CanId> ids;
    ids.reserve(callbacks_.size());
    for (const auto& [id, _] : callbacks_) ids.push_back(id);
    return ids;
}

void MessageHandler::set_default_callback(std::function<void(CanId, std::span<const uint8_t>)> cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    default_callback_ = std::move(cb);
}

} // namespace putm_ev_can
