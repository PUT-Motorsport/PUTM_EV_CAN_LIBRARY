/**
 * @file can_driver.cpp
 * @brief Implementation of the main CAN driver logic.
 */

#include "can_driver.hpp"

namespace putm_ev_can {

CanDriver::~CanDriver() {
    // Stop the RX thread / interrupts BEFORE members are destroyed,
    // otherwise the RX path may call into a destroyed handler_.
    Stop();
}

void CanDriver::Stop() {
    hal_.stop();
    hal_.set_rx_callback(nullptr);
}

bool CanDriver::Init(InitParamType param) {
    // 1. Initialize HAL (does not start reception yet)
#if defined(PUTM_CAN_BACKEND_STM32)
    if (!param) return false;
    hal_.set_handle(param);
#elif defined(PUTM_CAN_BACKEND_ROS2)
    SocketCanConfig cfg;
    cfg.interface = param;
    if (!hal_.open(cfg)) return false;
#endif
    return finish_init();
}

#if defined(PUTM_CAN_BACKEND_ROS2)
bool CanDriver::Init(const SocketCanConfig& config) {
    if (!hal_.open(config)) return false;
    return finish_init();
}
#endif

bool CanDriver::finish_init() {
    // 2. Register RX Callback - before reception starts (no race with the RX thread)
    hal_.set_rx_callback([this](const PUTM_CAN::CanFrame& frame) {
        std::span<const uint8_t> data(frame.data.data(), frame.dlc);
        handler_.handle_message(frame.id, data);
    });

    // 3. Configure Filters (Accept All)
    std::array<PUTM_CAN::CanFilter, 1> filters = {{
        { .id = 0, .mask = 0x000, .extended = false, .fifo = 0 }
    }};
    if (!hal_.configure_filters(filters)) {
        return false;
    }

    // 4. Start Hardware / RX thread
    return hal_.init();
}

PUTM_CAN::BusDiagnostics CanDriver::GetDiagnostics() const {
    return hal_.get_diagnostics();
}

} // namespace putm_ev_can
