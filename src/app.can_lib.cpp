/**
 * @file app_can_lib.cpp
 * @brief Implementation of the AppCAN facade.
 */

#include "app_can_lib.hpp"

// Note: CanHandleType depends on the selected STM32 family (defined in stm32_hal_selector.hpp)
bool AppCAN::Init(putm_ev_can::CanHandleType* hcan) {
    if (!hcan) return false;

    // 1. Configure Hardware Layer
    hal_.set_handle(hcan);

    // 2. Configure Message Handler
    handler_.set_locking_mechanism(
        []() { __disable_irq(); },
        []() { __enable_irq(); }
    );

    // 3. Configure Filters (Accept All)
    std::array<PUTM_CAN::CanFilter, 1> filters = {{
        { .id = 0, .mask = 0x000, .extended = false, .fifo = 0 }
    }};
    
    if (!interface_.configure_filters(filters)) {
        return false;
    }

    // 4. Start Interface
    return interface_.init();
}

void AppCAN::Poll() {
    // RX Processing is interrupt-driven.
    // This function only handles the diagnostic LED.
    if (led_config_.enabled) {
        handle_status_led();
    }
}

void AppCAN::handle_status_led() {
    auto status = hal_.get_bus_status();
    uint32_t now = HAL_GetTick();
    
    if (status == PUTM_CAN::BusStatus::BUS_OFF) {
        // ERROR: Steady ON
        HAL_GPIO_WritePin(led_config_.port, led_config_.pin, GPIO_PIN_SET);
    }
    else if (status == PUTM_CAN::BusStatus::OK) {
        // OK: Even Blink (1Hz)
        if ((now % 1000) < 500) {
            HAL_GPIO_WritePin(led_config_.port, led_config_.pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(led_config_.port, led_config_.pin, GPIO_PIN_RESET);
        }
    }
    else {
        // WARNING: Uneven Blink
        uint32_t cycle = now % 1000;
        bool state = false;
        if (cycle < 100) state = true;
        else if (cycle < 200) state = false;
        else if (cycle < 300) state = true;
        else state = false;
        
        HAL_GPIO_WritePin(led_config_.port, led_config_.pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}