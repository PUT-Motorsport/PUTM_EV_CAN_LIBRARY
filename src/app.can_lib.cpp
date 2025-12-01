/**
 * @file app_can_lib.cpp
 * @brief Implementation of the AppCAN facade.
 */

#include "app_can_lib.hpp"

bool AppCAN::Init(FDCAN_HandleTypeDef* hfdcan) {
    if (!hfdcan) return false;

    // 1. Configure Hardware Layer
    hal_.set_handle(hfdcan);

    // 2. Configure Message Handler (Thread Safety)
    handler_.set_locking_mechanism(
        []() { __disable_irq(); },
        []() { __enable_irq(); }
    );

    // 3. Configure Filters
    // Create a filter that accepts ALL messages (Mask 0x000)
    // Using std::array ensures allocation on stack, no heap usage.
    std::array<PUTM_CAN::CanFilter, 1> filters = {{
        { .id = 0, .mask = 0x000, .extended = false, .fifo = 0 }
    }};
    
    // Cast array to span automatically
    if (!interface_.configure_filters(filters)) {
        return false;
    }

    // 4. Start Interface (Initializes HAL and starts FDCAN)
    return interface_.init();
}

void AppCAN::Poll() {
    // Delegate to the interface implementation
    interface_.process_received_messages();
}