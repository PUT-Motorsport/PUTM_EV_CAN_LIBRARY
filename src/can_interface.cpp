/**
 * @file can_interface.cpp
 * @brief Default high-level CAN interface using HAL + MessageHandler.
 */

#include "can_interface.hpp"
#include "message_handler.hpp"
#include "can_hal.hpp"
#include <cstring> // std::memcpy

namespace putm_ev_can {

/**
 * @brief Default implementation that delegates TX/RX to HAL, and RX to MessageHandler.
 * AUTO_INIT = true -> init() wywoływane w konstruktorze (o ile HAL != nullptr).
 */
class DefaultCanInterface : public CanInterface {
public:
    DefaultCanInterface(PUTM_CAN::ICanHal* hal, MessageHandler& mh)
        : hal_(hal), handler_(mh) {
        if (hal_) hal_->init(); // AUTO_INIT = true
    }

    bool init() override {
        if (!hal_) return false;
        return hal_->init(); // idempotent na większości platform
    }

    bool is_ready() const override {
        return (hal_ != nullptr) && hal_->is_initialized();
    }

    void process_received_messages() override {
        if (!hal_) return;
        PUTM_CAN::CanFrame f;
        while (hal_->receive(f)) {
            handler_.handle_message_with_default(
                f.id,
                std::span<const uint8_t>(f.data.data(), f.dlc));
        }
    }

    bool configure_filters(const std::vector<PUTM_CAN::CanFilter>& filters) override {
        if (!hal_) return false;
        return hal_->configure_filters(filters);
    }

protected:
    bool send_raw(CanId id, std::span<const uint8_t> data) override {
        if (!hal_) return false;
        PUTM_CAN::CanFrame frame{};
        frame.id  = id;
        frame.dlc = static_cast<uint8_t>(data.size());
        std::memcpy(frame.data.data(), data.data(), frame.dlc);
        return hal_->transmit(frame);
    }

private:
    PUTM_CAN::ICanHal* hal_;
    MessageHandler&    handler_;
};

/* ===== Helper factory (opcjonalne) ===== */

/// Prosta fabryka do użycia w aplikacji (nieobowiązkowa).
CanInterface* make_default_interface(PUTM_CAN::ICanHal* hal, MessageHandler& mh) {
    return new DefaultCanInterface(hal, mh);
}

} // namespace putm_ev_can
