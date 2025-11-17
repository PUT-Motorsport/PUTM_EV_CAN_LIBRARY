/**
 * @file message_handler.cpp
 * @brief Message routing + callback management (Dependency Injection support).
 */

#include "message_handler.hpp"
#include <span>
#include <vector>

// Usuwamy iostream i exceptions dla czystego embedded
// Logika blokady jest teraz wewnatrz klasy ScopedLock zdefiniowanej w hpp

namespace putm_ev_can {

void MessageHandler::handle_message(CanId id, std::span<const uint8_t> data) {
    if (data.empty() || data.size() > 8) return;
    
    // Uzywamy wewnetrznej klasy ScopedLock (zdefiniowanej w .hpp)
    // Przekazujemy *this, zeby dostala sie do funkcji lock_fn_/unlock_fn_
    ScopedLock guard(*this); 
    
    auto it = callbacks_.find(id);
    if (it != callbacks_.end()) {
        // Bezposrednie wywolanie callbacka (bez try-catch)
        it->second(data);
    }
}

void MessageHandler::handle_message_with_default(CanId id, std::span<const uint8_t> data) {
    if (data.empty() || data.size() > 8) return;
    
    ScopedLock guard(*this);
    auto it = callbacks_.find(id);
    if (it != callbacks_.end()) {
        it->second(data);
    } else if (default_callback_) {
        default_callback_(id, data);
    }
}

void MessageHandler::handle_messages_batch(
    const std::vector<std::pair<CanId, std::span<const uint8_t>>>& messages) {
    
    ScopedLock guard(*this);
    for (const auto& [id, data] : messages) {
        if (data.size() > 8 || data.empty()) continue;
        
        auto it = callbacks_.find(id);
        if (it != callbacks_.end()) {
            it->second(data);
        } else if (default_callback_) {
            default_callback_(id, data);
        }
    }
}

bool MessageHandler::has_callback(CanId id) const {
    // const_cast jest potrzebny, bo ScopedLock wymaga dostepu do metod (chociaz lock nie zmienia stanu logicznego)
    // W tym wypadku bezpieczniej jest uzyc mutable na wskaznikach funkcji, co zrobilismy w hpp.
    // Tutaj po prostu rzutujemy, poniewaz nasze lock_fn_ nie zmieniaja stanu obiektu handlera sensu stricte.
    ScopedLock guard(const_cast<MessageHandler&>(*this));
    return callbacks_.find(id) != callbacks_.end();
}

size_t MessageHandler::callback_count() const {
    ScopedLock guard(const_cast<MessageHandler&>(*this));
    return callbacks_.size();
}

void MessageHandler::clear_callbacks() {
    ScopedLock guard(*this);
    callbacks_.clear();
}

bool MessageHandler::remove_callback(CanId id) {
    ScopedLock guard(*this);
    return callbacks_.erase(id) > 0;
}

std::vector<CanId> MessageHandler::get_registered_ids() const {
    ScopedLock guard(const_cast<MessageHandler&>(*this));
    std::vector<CanId> ids;
    ids.reserve(callbacks_.size());
    for (const auto& [id, _] : callbacks_) ids.push_back(id);
    return ids;
}

void MessageHandler::set_default_callback(std::function<void(CanId, std::span<const uint8_t>)> cb) {
    ScopedLock guard(*this);
    default_callback_ = std::move(cb);
}

} // namespace putm_ev_can