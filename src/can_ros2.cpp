/**
 * @file can_ros2.cpp
 * @brief Minimal SocketCAN/ROS2 HAL stub (soft-filtering in upper layer).
 *
 * UWAGA: to jest minimalny szkic. Jeśli masz uchwyt socketa, podmień
 * implementacje transmit/receive/configure_filters na realne.
 */

#include "can_hal.hpp"

namespace putm_ev_can {

class Ros2CanHal : public PUTM_CAN::ICanHal {
public:
    bool init() override { initialized_ = true; return true; }

    bool transmit(const PUTM_CAN::CanFrame& /*frame*/) override {
        // TODO: wyślij przez SocketCAN
        return true;
    }

    bool receive(PUTM_CAN::CanFrame& /*frame*/) override {
        // TODO: odbierz z SocketCAN (non-blocking)
        return false;
    }

    bool configure_filters(const std::vector<PUTM_CAN::CanFilter>& /*filters*/) override {
        // TODO: setsockopt(CAN_RAW_FILTER). Na razie OK -> soft filtering wyżej.
        return true;
    }

    bool is_initialized() const override { return initialized_; }

private:
    bool initialized_ = false;
};

} // namespace putm_ev_can
