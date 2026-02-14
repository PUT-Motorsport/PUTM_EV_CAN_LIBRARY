/**
 * @file can_driver.cpp
 * @brief Implementation of the main CAN driver logic.
 */

#include "PUTM_EV_CAN_LIBRARY/include/can_driver.hpp"

namespace putm_ev_can {

CanDriver::~CanDriver() {
    // Prevent calling into destroyed object by unregistering callback
    hal_.set_rx_callback(nullptr);
}

bool CanDriver::Init(InitParamType param) {
    // 1. Initialize HAL
#if defined(PUTM_CAN_BACKEND_STM32)
    if (!param) return false;
    hal_.set_handle(param);
#elif defined(PUTM_CAN_BACKEND_ROS2)
    if (!hal_.init(param)) return false;
#endif

    // 2. Register RX Callback
    hal_.set_rx_callback([this](const PUTM_CAN::CanFrame& frame) {
        // Pass frame to router
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

    // 4. Start Hardware
    return hal_.init();
}

PUTM_CAN::BusDiagnostics CanDriver::GetDiagnostics() const {
    return hal_.get_diagnostics();
}

} // namespace putm_ev_can