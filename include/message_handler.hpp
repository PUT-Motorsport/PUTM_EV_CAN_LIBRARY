/**
 * @file message_handler.hpp
 * @brief Type-safe message callbacks + routing (no switch-case).
 */

#ifndef PUTM_EV_CAN_MESSAGE_HANDLER_HPP
#define PUTM_EV_CAN_MESSAGE_HANDLER_HPP

#include <functional>
#include <span>
#include <unordered_map>
#include <vector>
#include <utility>
#include <mutex>
#include "can_interface.hpp" // provides find_dbc_entry + DBC registry
#include "PUTM_CAN_1.h"

namespace putm_ev_can {

using CanId = uint32_t;

/**
 * @class MessageHandler
 * @brief Thread-safe callback registry and routing based on DBC.
 */
class MessageHandler {
public:
    MessageHandler() = default;

    template <typename MsgType>
    void register_callback(CanId id, std::function<void(const MsgType&)> callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_[id] = [id, callback](std::span<const uint8_t> data) {
            MsgType msg{};
            if (decode_to_message<MsgType>(id, data, msg)) {
                callback(msg);
            }
        };
    }

    void handle_message(CanId id, std::span<const uint8_t> data);
    void handle_message_with_default(CanId id, std::span<const uint8_t> data);
    void handle_messages_batch(const std::vector<std::pair<CanId, std::span<const uint8_t>>>& messages);

    bool has_callback(CanId id) const;
    size_t callback_count() const;
    void clear_callbacks();
    bool remove_callback(CanId id);
    std::vector<CanId> get_registered_ids() const;

    void set_default_callback(std::function<void(CanId, std::span<const uint8_t>)> callback);

private:
    std::unordered_map<CanId, std::function<void(std::span<const uint8_t>)>> callbacks_;
    std::function<void(CanId, std::span<const uint8_t>)> default_callback_;
    mutable std::mutex mutex_;

    template <typename MsgType>
    static bool decode_to_message(CanId id, std::span<const uint8_t> data, MsgType& msg) {
        const auto* e = find_dbc_entry(id);
        if (!e) return false;
        if (data.size() != e->len) return false;
        return e->unpack(&msg, data.data(), data.size()) >= 0;
    }
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_MESSAGE_HANDLER_HPP
