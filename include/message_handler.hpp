/**
 * @file message_handler.hpp
 * @brief Message callback manager and router.
 * @details Optimized for embedded systems: uses a static array instead of the heap.
 */

#ifndef PUTM_EV_CAN_MESSAGE_HANDLER_HPP
#define PUTM_EV_CAN_MESSAGE_HANDLER_HPP

#include <functional>
#include <span>
#include <array>
#include <algorithm>
#include "can_interface.hpp" 
#include "PUTM_CAN_M.h"

namespace putm_ev_can {

// Import CanId from HAL namespace
using PUTM_CAN::CanId;

/**
 * @brief Maximum number of registered callbacks.
 */
constexpr size_t MAX_CALLBACKS = 32;

/**
 * @brief Class managing message distribution to registered callbacks.
 */
class MessageHandler {
public:
    MessageHandler() = default;

    /**
     * @brief Configures the interrupt locking mechanism.
     * @param lock_fn Function to disable interrupts.
     * @param unlock_fn Function to enable interrupts.
     */
    void set_locking_mechanism(std::function<void()> lock_fn, std::function<void()> unlock_fn) {
        lock_fn_ = lock_fn;
        unlock_fn_ = unlock_fn;
    }

    /**
     * @brief Registers a callback for a specific ID.
     * @return true if successful, false if MAX_CALLBACKS reached.
     */
    template <typename MsgType>
    bool register_callback(CanId id, std::function<void(const MsgType&)> callback) {
        ScopedLock guard(*this);
        
        // 1. Update existing
        for (auto& entry : callbacks_) {
            if (entry.active && entry.id == id) {
                entry.callback = [id, callback](std::span<const uint8_t> data) {
                    MsgType msg{};
                    if (decode_to_message<MsgType>(id, data, msg)) {
                        callback(msg);
                    }
                };
                return true;
            }
        }

        // 2. Find empty slot
        for (auto& entry : callbacks_) {
            if (!entry.active) {
                entry.id = id;
                entry.active = true;
                entry.callback = [id, callback](std::span<const uint8_t> data) {
                    MsgType msg{};
                    if (decode_to_message<MsgType>(id, data, msg)) {
                        callback(msg);
                    }
                };
                return true;
            }
        }
        return false; 
    }

    void handle_message_with_default(CanId id, std::span<const uint8_t> data);

private:
    std::function<void()> lock_fn_ = nullptr;
    std::function<void()> unlock_fn_ = nullptr;

    struct CallbackEntry {
        bool active = false;
        CanId id = 0;
        std::function<void(std::span<const uint8_t>)> callback;
    };

    std::array<CallbackEntry, MAX_CALLBACKS> callbacks_;
    std::function<void(CanId, std::span<const uint8_t>)> default_callback_;

    // RAII Lock
    class ScopedLock {
    public:
        explicit ScopedLock(MessageHandler& mh) : mh_(mh) { if (mh_.lock_fn_) mh_.lock_fn_(); }
        ~ScopedLock() { if (mh_.unlock_fn_) mh_.unlock_fn_(); }
    private:
        MessageHandler& mh_;
    };

    template <typename MsgType>
    static bool decode_to_message(CanId id, std::span<const uint8_t> data, MsgType& msg) {
        // Now visible due to can_interface.hpp include
        const auto* e = find_dbc_entry(id);
        if (!e || data.size() != e->len) return false;
        return e->unpack(&msg, data.data(), data.size()) >= 0;
    }
    
    friend class ScopedLock;
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_MESSAGE_HANDLER_HPP