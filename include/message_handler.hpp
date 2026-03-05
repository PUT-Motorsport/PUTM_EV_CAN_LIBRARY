/**
 * @file message_handler.hpp
 * @brief Thread-safe message routing logic.
 */

#ifndef PUTM_EV_CAN_MESSAGE_HANDLER_HPP
#define PUTM_EV_CAN_MESSAGE_HANDLER_HPP

#include <functional>
#include <array>
#include <span>
#include "can_hal.hpp"
#include "utils/critical_section.hpp"

namespace putm_ev_can {

// Type definition for DBC entries (used by Driver and Handler)
struct DbcEntry {
    PUTM_CAN::CanId id;
    uint8_t len;
    void* pack;   
    void* unpack; 
};

// External declaration (implementation provided by generated code)
extern const DbcEntry* find_dbc_entry(PUTM_CAN::CanId id);

class MessageHandler {
public:
    static constexpr size_t MAX_CALLBACKS = 32;

    template <typename MsgType>
    bool register_callback(PUTM_CAN::CanId id, std::function<void(const MsgType&)> callback) {
        LockGuard lock(protection_); // Thread/IRQ Safe
        
        // 1. Update existing slot
        for (auto& entry : callbacks_) {
            if (entry.active && entry.id == id) {
                entry.callback = create_unpacker<MsgType>(id, callback);
                return true;
            }
        }

        // 2. Find new empty slot
        for (auto& entry : callbacks_) {
            if (!entry.active) {
                entry.id = id;
                entry.active = true;
                entry.callback = create_unpacker<MsgType>(id, callback);
                return true;
            }
        }
        return false; 
    }

    void handle_message(PUTM_CAN::CanId id, std::span<const uint8_t> data);

private:
    struct CallbackEntry {
        bool active = false;
        PUTM_CAN::CanId id = 0;
        std::function<void(std::span<const uint8_t>)> callback;
    };

    std::array<CallbackEntry, MAX_CALLBACKS> callbacks_;
    CriticalSection protection_;

    // Helper to create type-safe unpacking lambda
    template <typename MsgType>
    auto create_unpacker(PUTM_CAN::CanId id, std::function<void(const MsgType&)> callback) {
        return [id, callback](std::span<const uint8_t> data) {
            const auto* e = find_dbc_entry(id);
            if (!e) return;
            
            MsgType msg{};
            using UnpackFn = int(*)(void*, const uint8_t*, size_t);
            auto unpack_fn = reinterpret_cast<UnpackFn>(e->unpack);

            if (unpack_fn(&msg, data.data(), data.size()) >= 0) {
                callback(msg);
            }
        };
    }
};

} // namespace putm_ev_can

#endif