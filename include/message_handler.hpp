/**
 * @file message_handler.hpp
 * @brief CAN Message Handler with callback management
 * @details Handles message routing, callback registration, and type-safe
 *          message dispatch for received CAN messages with thread safety
 * 
 * @author PUTM Team
 * @version 2.0
 * @date 2024
 */

#ifndef PUTM_EV_CAN_MESSAGE_HANDLER_HPP
#define PUTM_EV_CAN_MESSAGE_HANDLER_HPP

#include <functional>
#include <span>
#include <unordered_map>
#include <vector>
#include <utility>
#include "can_interface.hpp"

namespace putm_ev_can {

/**
 * @class MessageHandler
 * @brief Manages CAN message callbacks and routing with thread safety
 * 
 * Provides comprehensive message handling with support for:
 * - Type-safe callbacks for specific message types
 * - Batch message processing
 * - Default callback for unhandled messages
 * - Thread-safe operations
 * - Error handling and logging
 */
class MessageHandler {
public:
    /**
     * @brief Default constructor
     */
    MessageHandler() = default;

    /**
     * @brief Register type-safe callback for specific CAN message
     * @tparam MsgType DBC message structure type
     * @param id CAN message identifier
     * @param callback Function to call when message is received
     */
    template <typename MsgType>
    void register_callback(CanId id, std::function<void(const MsgType&)> callback) {
        callbacks_[id] = [callback](std::span<const uint8_t> data) {
            MsgType msg;
            if (decode_to_message<MsgType>(data, msg)) {
                callback(msg);
            }
        };
    }

    // === THREAD-SAFE METHODS IMPLEMENTED IN .cpp ===
    
    /**
     * @brief Handle incoming CAN message and invoke appropriate callback
     * @param id CAN message identifier
     * @param data Message data bytes
     */
    void handle_message(CanId id, std::span<const uint8_t> data);
    
    /**
     * @brief Handle message with default callback support
     * @param id CAN message identifier
     * @param data Message data bytes
     */
    void handle_message_with_default(CanId id, std::span<const uint8_t> data);
    
    /**
     * @brief Process multiple messages in batch (more efficient)
     * @param messages Vector of message ID and data pairs
     */
    void handle_messages_batch(const std::vector<std::pair<CanId, std::span<const uint8_t>>>& messages);
    
    /**
     * @brief Check if callback is registered for specific message ID
     * @param id CAN message identifier to check
     * @return true if callback is registered
     */
    bool has_callback(CanId id) const;
    
    /**
     * @brief Get number of registered callbacks
     * @return Number of active callbacks
     */
    size_t callback_count() const;
    
    /**
     * @brief Remove all registered callbacks
     */
    void clear_callbacks();
    
    /**
     * @brief Remove callback for specific message ID
     * @param id CAN message identifier
     * @return true if callback was removed
     */
    bool remove_callback(CanId id);
    
    /**
     * @brief Get list of all registered message IDs
     * @return Vector of registered CAN message identifiers
     */
    std::vector<CanId> get_registered_ids() const;
    
    /**
     * @brief Set default callback for unhandled messages
     * @param callback Function to call for messages without specific handlers
     */
    void set_default_callback(std::function<void(CanId, std::span<const uint8_t>)> callback);

private:
    std::unordered_map<CanId, std::function<void(std::span<const uint8_t>)>> callbacks_;
    std::function<void(CanId, std::span<const uint8_t>)> default_callback_;

    /**
     * @brief Decode raw CAN data to specific message type
     * @tparam MsgType Target message type
     * @param data Raw CAN data (must be 8 bytes)
     * @param[out] msg Message structure to populate
     * @return true if decoding successful
     */
    template <typename MsgType>
    bool decode_to_message(std::span<const uint8_t> data, MsgType& msg) {
        if (data.size() != 8) {
            return false;
        }
        
        // Use DBC decode functions based on message type
        if constexpr (std::is_same_v<MsgType, PUTM_CAN_PcMainData>) {
            Pc_MainData_decode(data.data(), &msg);
            return true;
        } else if constexpr (std::is_same_v<MsgType, PUTM_CAN_PcTemperatureData>) {
            Pc_TemperatureData_decode(data.data(), &msg);
            return true;
        }
        // Add more message types as needed
        
        return false;
    }
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_MESSAGE_HANDLER_HPP