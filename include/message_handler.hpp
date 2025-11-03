/**
 * @file message_handler.hpp
 * @brief Manages CAN message callbacks with DBC-generated types.
 */

#ifndef PUTM_CAN_MESSAGE_HANDLER_HPP
#define PUTM_CAN_MESSAGE_HANDLER_HPP

#include <functional>
#include <span>
#include <unordered_map>

namespace putm_can {

using CanId = uint32_t;

/**
 * @brief Central message dispatcher.
 *
 * Stores callbacks per CAN ID and invokes them with unpacked messages.
 */
class MessageHandler {
public:
    /// Default constructor
    MessageHandler() = default;

    /**
     * @brief Registers a callback for a CAN ID.
     *
     * @tparam MsgType DBC-generated message type
     * @param id CAN message ID
     * @param callback Function to call with unpacked message
     */
    template <typename MsgType>
    void register_callback(CanId id, std::function<void(const MsgType&)> callback) {
        callbacks_[id] = [callback](std::span<const uint8_t> data) {
            MsgType msg{};
            if (MsgType##_unpack(&msg, data.data(), data.size()) == 0) {
                callback(msg);
            }
        };
    }

    /**
     * @brief Dispatches received raw data to registered callback.
     * @param id CAN ID
     * @param data Raw payload
     */
    void handle_message(CanId id, std::span<const uint8_t> data) {
        auto it = callbacks_.find(id);
        if (it != callbacks_.end()) {
            it->second(data);
        }
    }

private:
    /// Map of CAN ID → raw data callback
    std::unordered_map<CanId, std::function<void(std::span<const uint8_t>)>> callbacks_;
};

} // namespace putm_can

#endif // PUTM_CAN_MESSAGE_HANDLER_HPP