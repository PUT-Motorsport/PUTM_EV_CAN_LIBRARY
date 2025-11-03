/**
 * @file can_interface.hpp
 * @brief CAN Interface for STM32 FDCAN with DBC support
 * @details Provides hardware-agnostic CAN interface using DBC-generated structures
 * 
 * @author PUTM Team
 * @version 2.0
 * @date 2024
 */

#ifndef PUTM_EV_CAN_INTERFACE_HPP
#define PUTM_EV_CAN_INTERFACE_HPP

#include <cstdint>
#include <functional>
#include <span>
#include <unordered_map>
#include "can_message_defs.h"

namespace putm_ev_can {

/// CAN message identifier type
using CanId = uint32_t;

/**
 * @typedef CanId
 * @brief CAN message identifier (11-bit or 29-bit)
 */
using CanId = uint32_t;

/**
 * @class CanInterface
 * @brief Abstract CAN interface using DBC message definitions
 * 
 * This class provides a unified interface for CAN communication across
 * different hardware platforms (STM32, ROS2) using DBC-generated structures.
 */
class CanInterface {
public:
    virtual ~CanInterface() = default;

    /**
     * @brief Initialize CAN hardware interface
     * @return true if initialization successful
     * @return false if hardware initialization failed
     */
    virtual bool init() = 0;

    /**
     * @brief Send CAN message using DBC structure
     * @tparam MsgType DBC message structure type
     * @param id CAN message ID
     * @param msg DBC message structure to send
     * @return true if message sent successfully
     * @return false if encoding or transmission failed
     * 
     * @example
     * @code
     * PUTM_CAN_PcMainData main_data = {0};
     * main_data.vehicleSpeed = 100;
     * can_interface.send(0x10, main_data);
     * @endcode
     */
    template<typename MsgType>
    bool send(CanId id, const MsgType& msg) {
        uint8_t buffer[8];
        
        // Encode message using DBC functions
        if (!encode_message(id, msg, buffer)) {
            return false;
        }
        
        return send_raw(id, std::span<const uint8_t>(buffer, 8));
    }

    /**
     * @brief Register callback for received CAN messages
     * @tparam MsgType DBC message structure type
     * @param id CAN message ID to listen for
     * @param callback Function to call when message is received
     * 
     * @note Callbacks are executed during process_received_messages() call
     */
    template<typename MsgType>
    void register_callback(CanId id, std::function<void(const MsgType&)> callback) {
        callbacks_[id] = [callback](std::span<const uint8_t> data) {
            MsgType msg;
            if (decode_message(data, msg)) {
                callback(msg);
            }
        };
    }

    /**
     * @brief Process all received messages and trigger callbacks
     * @details This method should be called periodically from main loop
     *          to handle incoming CAN messages
     */
    virtual void process_received_messages() = 0;

    /**
     * @brief Check if CAN interface is operational
     * @return true if interface is initialized and ready
     * @return false if interface is in error state
     */
    virtual bool is_ready() const = 0;

protected:
    /**
     * @brief Send raw CAN data (to be implemented by derived classes)
     * @param id CAN message ID
     * @param data Raw data bytes (max 8 bytes)
     * @return true if transmission successful
     */
    virtual bool send_raw(CanId id, std::span<const uint8_t> data) = 0;

private:
    std::unordered_map<CanId, std::function<void(std::span<const uint8_t>)>> callbacks_;

    /**
     * @brief Encode DBC message to CAN data bytes
     * @tparam MsgType DBC message structure type
     * @param id CAN message ID
     * @param msg Message structure to encode
     * @param[out] buffer Output buffer (8 bytes)
     * @return true if encoding successful
     */
    template<typename MsgType>
    bool encode_message(CanId id, const MsgType& msg, uint8_t* buffer) {
        // Map CAN ID to appropriate encode function
        switch(id) {
            case 0x10:
                Pc_MainData_encode(buffer, reinterpret_cast<const PUTM_CAN_PcMainData*>(&msg));
                return true;
            case 0x60:
                Pc_TemperatureData_encode(buffer, reinterpret_cast<const PUTM_CAN_PcTemperatureData*>(&msg));
                return true;
            // Add more cases for other message IDs
            default:
                return false;
        }
    }

    /**
     * @brief Decode CAN data bytes to DBC message
     * @tparam MsgType DBC message structure type
     * @param data Input data bytes
     * @param[out] msg Message structure to populate
     * @return true if decoding successful
     */
    template<typename MsgType>
    bool decode_message(std::span<const uint8_t> data, MsgType& msg) {
        // This would need message type detection or separate registration
        // For now, simplified implementation
        if (data.size() != 8) {
            return false;
        }
        
        // In real implementation, you'd need to know the message type
        // This is a simplified version
        return true;
    }
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_INTERFACE_HPP