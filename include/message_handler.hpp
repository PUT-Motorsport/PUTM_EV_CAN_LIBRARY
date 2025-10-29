#ifndef PUTM_CAN_MESSAGE_HANDLER_HPP
#define PUTM_CAN_MESSAGE_HANDLER_HPP

#include <functional>
#include <span>
#include <unordered_map>
#include "can_interface.hpp"
#include "generated/pdu.pb.h" // Generated Protobuf file

namespace putm_can {

/**
 * @brief Type alias for CAN message ID.
 */
using CanId = uint32_t;

/**
 * @brief Type alias for callback function.
 */
using CallbackFunction = std::function<void(const google::protobuf::Message&)>;

/**
 * @brief Class managing CAN messages, handling serialization/deserialization and callbacks.
 * 
 * This class is part of the communication abstraction layer and works with CanInterface.
 * It uses Protobuf for message serialization/deserialization.
 */
class MessageHandler {
public:
    /**
     * @brief Default constructor.
     */
    MessageHandler();

    /**
     * @brief Registers a callback for a specific CAN ID.
     * @param id The CAN message ID.
     * @param callback The callback function to invoke upon message reception.
     * @tparam MsgType The Protobuf message type (e.g., PduChannel).
     */
    template <typename MsgType>
    void register_callback(CanId id, std::function<void(const MsgType&)> callback);

    /**
     * @brief Serializes a message into a CAN buffer (max 8 bytes).
     * @param msg The Protobuf message to serialize.
     * @param buffer The output buffer (span<uint8_t>).
     * @return true if serialization succeeded, false otherwise.
     */
    template <typename MsgType>
    bool serialize(const MsgType& msg, std::span<uint8_t> buffer) const;

    /**
     * @brief Deserializes a message from a CAN buffer.
     * @param id The CAN message ID.
     * @param buffer The input buffer (span<const uint8_t>).
     * @param[out] msg The Protobuf message object to fill.
     * @return true if deserialization succeeded, false otherwise.
     */
    template <typename MsgType>
    bool deserialize(CanId id, std::span<const uint8_t> buffer, MsgType& msg) const;

    /**
     * @brief Invokes the callback for a received message.
     * @param id The CAN message ID.
     * @param data Raw data from the CAN buffer.
     */
    void handle_message(CanId id, std::span<const uint8_t> data);

private:
    std::unordered_map<CanId, CallbackFunction> callbacks_;  /**< Map storing callbacks for specific IDs. */

    static constexpr size_t CAN_MAX_DLC = 8;                 /**< Maximum CAN message size (8 bytes for standard). */
};

} // namespace putm_can

#endif // PUTM_CAN_MESSAGE_HANDLER_HPP