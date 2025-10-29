#ifndef PUTM_CAN_INTERFACE_HPP
#define PUTM_CAN_INTERFACE_HPP

#include <cstdint>
#include <functional>
#include <span>  // C++20 for buffers
#include "generated/can_ids.pb.h"  // Enum from proto

namespace putm_can {

/**
 * @brief Abstract base class for CAN interface.
 * 
 * Defines the basic operations for sending and receiving CAN messages using Protobuf templates.
 */
class CanInterface {
public:
    virtual ~CanInterface() = default;  /**< Virtual destructor for proper cleanup. */

    /**
     * @brief Sends a CAN message with automatic Protobuf serialization.
     * 
     * Serializes the Protobuf message into a CAN frame and transmits it.
     * @tparam MsgType The Protobuf message type (e.g., PcMainData).
     * @param id The CAN message ID.
     * @param msg Reference to the Protobuf message to send.
     * @return true if the send operation succeeded, false otherwise.
     */
    template <typename MsgType>
    bool send(CanId id, const MsgType& msg) {
        uint8_t buffer[8];
        std::span<uint8_t> buf_span(buffer);
        if (!serialize(msg, buf_span)) return false;
        return send_raw(id, buf_span);
    }

    /**
     * @brief Registers a callback for received messages.
     * 
     * Sets up a callback function to be invoked when a message with the specified ID is received.
     * @tparam MsgType The Protobuf message type.
     * @param id The CAN message ID.
     * @param cb Callback function to handle the deserialized message.
     */
    template <typename MsgType>
    void register_callback(CanId id, std::function<void(const MsgType&)> cb) {
        register_raw_callback(id, [cb](std::span<const uint8_t> data) {
            MsgType msg;
            if (deserialize(data, msg)) cb(msg);
        });
    }

protected:
    /**
     * @brief Sends a raw CAN frame (to be implemented by derived classes).
     * @param id The CAN message ID.
     * @param data Raw data span to send.
     * @return true if the send operation succeeded, false otherwise.
     */
    virtual bool send_raw(CanId id, std::span<const uint8_t> data) = 0;

    /**
     * @brief Registers a raw callback for received data.
     * @param id The CAN message ID.
     * @param cb Callback function for raw data.
     */
    virtual void register_raw_callback(CanId id, std::function<void(std::span<const uint8_t>)> cb) = 0;

    /**
     * @brief Serializes a Protobuf message into a buffer.
     * @tparam MsgType The Protobuf message type.
     * @param msg The message to serialize.
     * @param buffer The output buffer.
     * @return true if serialization succeeded, false otherwise.
     */
    template <typename MsgType>
    static bool serialize(const MsgType& msg, std::span<uint8_t> buffer);

    /**
     * @brief Deserializes a Protobuf message from a buffer.
     * @tparam MsgType The Protobuf message type.
     * @param data The input raw data.
     * @param[out] msg The message object to fill.
     * @return true if deserialization succeeded, false otherwise.
     */
    template <typename MsgType>
    static bool deserialize(std::span<const uint8_t> data, MsgType& msg);
};

}  // namespace putm_can

#endif