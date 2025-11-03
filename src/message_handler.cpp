/**
 * @file message_handler.cpp
 * @brief Message Handler implementation
 * @details Provides implementation for message routing and callback management
 *          including thread safety and error handling
 * 
 * @author PUTM Team
 * @version 2.0
 * @date 2024
 */

#include "message_handler.hpp"
#include "can_message_defs.h"  // DBC generated headers
#include <iostream>
#include <mutex>

namespace putm_ev_can {

// Static mutex for thread safety
static std::mutex callback_mutex;

/**
 * @brief Get string representation of message ID for logging
 * @param id CAN message identifier
 * @return String describing the message
 */
static const char* message_id_to_string(CanId id) {
    switch(id) {
        case 0x100: return "PC_MainData";
        case 0x180: return "PC_TemperatureData";
        case 0x200: return "DriverInput";
        case 0x300: return "PDU_Data";
        default: return "Unknown";
    }
}

/**
 * @brief Handle incoming CAN message with proper error checking
 * @param id CAN message identifier
 * @param data Message data bytes
 * 
 * @note This method is thread-safe
 */
void MessageHandler::handle_message(CanId id, std::span<const uint8_t> data) {
    // Validate input parameters
    if (data.empty() || data.size() > 8) {
        // Log error or handle invalid data size
        return;
    }

    std::lock_guard<std::mutex> lock(callback_mutex);
    
    auto it = callbacks_.find(id);
    if (it != callbacks_.end()) {
        try {
            it->second(data);
        } catch (const std::exception& e) {
            // Handle callback exceptions gracefully
            std::cerr << "Callback error for message 0x" << std::hex << id 
                      << " (" << message_id_to_string(id) << "): " << e.what() << std::endl;
        }
    }
}

/**
 * @brief Remove all registered callbacks (thread-safe)
 */
void MessageHandler::clear_callbacks() {
    std::lock_guard<std::mutex> lock(callback_mutex);
    callbacks_.clear();
}

/**
 * @brief Remove callback for specific message ID (thread-safe)
 * @param id CAN message identifier
 * @return true if callback was removed
 * @return false if no callback was registered for the ID
 */
bool MessageHandler::remove_callback(CanId id) {
    std::lock_guard<std::mutex> lock(callback_mutex);
    return callbacks_.erase(id) > 0;
}

/**
 * @brief Get list of all registered message IDs
 * @return Vector of registered CAN message identifiers
 */
std::vector<CanId> MessageHandler::get_registered_ids() const {
    std::lock_guard<std::mutex> lock(callback_mutex);
    std::vector<CanId> ids;
    ids.reserve(callbacks_.size());
    
    for (const auto& pair : callbacks_) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

/**
 * @brief Check if callback is registered for specific message ID (thread-safe)
 * @param id CAN message identifier to check
 * @return true if callback is registered
 */
bool MessageHandler::has_callback(CanId id) const {
    std::lock_guard<std::mutex> lock(callback_mutex);
    return callbacks_.find(id) != callbacks_.end();
}

/**
 * @brief Get number of registered callbacks (thread-safe)
 * @return Number of active callbacks
 */
size_t MessageHandler::callback_count() const {
    std::lock_guard<std::mutex> lock(callback_mutex);
    return callbacks_.size();
}

/**
 * @brief Process multiple messages in batch
 * @param messages Vector of message ID and data pairs
 * 
 * @note More efficient than processing messages individually
 */
void MessageHandler::handle_messages_batch(const std::vector<std::pair<CanId, std::span<const uint8_t>>>& messages) {
    std::lock_guard<std::mutex> lock(callback_mutex);
    
    for (const auto& [id, data] : messages) {
        if (data.size() != 8) continue; // Skip invalid messages
        
        auto it = callbacks_.find(id);
        if (it != callbacks_.end()) {
            try {
                it->second(data);
            } catch (const std::exception& e) {
                std::cerr << "Batch callback error for message 0x" << std::hex << id 
                          << ": " << e.what() << std::endl;
            }
        }
    }
}

/**
 * @brief Set a default callback for unhandled messages
 * @param callback Function to call for messages without specific handlers
 */
void MessageHandler::set_default_callback(std::function<void(CanId, std::span<const uint8_t>)> callback) {
    std::lock_guard<std::mutex> lock(callback_mutex);
    default_callback_ = std::move(callback);
}

/**
 * @brief Handle message with default callback support
 * @param id CAN message identifier
 * @param data Message data bytes
 */
void MessageHandler::handle_message_with_default(CanId id, std::span<const uint8_t> data) {
    if (data.empty() || data.size() > 8) return;

    std::lock_guard<std::mutex> lock(callback_mutex);
    
    auto it = callbacks_.find(id);
    if (it != callbacks_.end()) {
        try {
            it->second(data);
        } catch (const std::exception& e) {
            std::cerr << "Callback error for message 0x" << std::hex << id << ": " << e.what() << std::endl;
        }
    } else if (default_callback_) {
        // No specific handler, use default callback if set
        try {
            default_callback_(id, data);
        } catch (const std::exception& e) {
            std::cerr << "Default callback error for message 0x" << std::hex << id << ": " << e.what() << std::endl;
        }
    }
}

} // namespace putm_ev_can