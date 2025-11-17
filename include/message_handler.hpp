/**
 * @file message_handler.hpp
 * @brief Type-safe message callbacks + routing (Dependency Injection for locking).
 */

#ifndef PUTM_EV_CAN_MESSAGE_HANDLER_HPP
#define PUTM_EV_CAN_MESSAGE_HANDLER_HPP

#include <functional>
#include <span>
#include <unordered_map>
#include <vector>
#include <utility>

#include "can_interface.hpp" 
#include "PUTM_CAN_1.h"

namespace putm_ev_can {

using CanId = uint32_t;

class MessageHandler {
public:
    MessageHandler() = default;

    /**
     * @brief Konfiguruje mechanizm blokowania (np. wylaczanie przerwan).
     * Wywolaj to w main.c przed uzyciem biblioteki.
     */
    void set_locking_mechanism(std::function<void()> lock_fn, std::function<void()> unlock_fn) {
        lock_fn_ = lock_fn;
        unlock_fn_ = unlock_fn;
    }

    template <typename MsgType>
    void register_callback(CanId id, std::function<void(const MsgType&)> callback) {
        ScopedLock guard(*this); // Automatyczny lock/unlock
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
    // Funkcje wstrzykiwane przez uzytkownika
    std::function<void()> lock_fn_ = nullptr;
    std::function<void()> unlock_fn_ = nullptr;

    // Wewnetrzna klasa RAII do obslugi blokady
    class ScopedLock {
    public:
        explicit ScopedLock(MessageHandler& mh) : mh_(mh) {
            if (mh_.lock_fn_) mh_.lock_fn_();
        }
        ~ScopedLock() {
            if (mh_.unlock_fn_) mh_.unlock_fn_();
        }
        ScopedLock(const ScopedLock&) = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;
    private:
        MessageHandler& mh_;
    };

    std::unordered_map<CanId, std::function<void(std::span<const uint8_t>)>> callbacks_;
    std::function<void(CanId, std::span<const uint8_t>)> default_callback_;

    template <typename MsgType>
    static bool decode_to_message(CanId id, std::span<const uint8_t> data, MsgType& msg) {
        const auto* e = find_dbc_entry(id);
        if (!e) return false;
        if (data.size() != e->len) return false;
        return e->unpack(&msg, data.data(), data.size()) >= 0;
    }
    
    // Przyjazn dla ScopedLock, zeby mial dostep do lock_fn_
    friend class ScopedLock;
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_MESSAGE_HANDLER_HPP