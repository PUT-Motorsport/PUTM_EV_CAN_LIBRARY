/**
 * @file app_can_lib.cpp
 * @brief Implementation of AppCAN facade for STM32 and ROS2.
 */

#include "PUTM_EV_CAN_LIBRARY/include/app_can_lib.hpp"

// --- INIT IMPLEMENTATION ---
bool AppCAN::Init(InitParamType param) {
    
    // 1. Konfiguracja Warstwy Sprzętowej (HAL)
#if defined(PUTM_CAN_BACKEND_STM32)
    if (!param) return false;
    hal_.set_handle(param);
    
    // Na STM32 musimy blokować przerwania podczas rejestracji callbacków
    handler_.set_locking_mechanism(
        []() { __disable_irq(); },
        []() { __enable_irq(); }
    );

#elif defined(PUTM_CAN_BACKEND_ROS2)
    // Na Linuxie inicjalizujemy socket (np. "can0")
    if (!hal_.init(param)) return false;
    
    // Na Linuxie nie wyłączamy przerwań systemowych ;)
    // Jeśli używasz wielu wątków w ROS, MessageHandler jest bezpieczny (ma własny mutex/lock)
#endif

    // 2. Konfiguracja Filtrów (Wspólna)
    // Domyślnie: Akceptuj wszystko
    std::array<PUTM_CAN::CanFilter, 1> filters = {{
        { .id = 0, .mask = 0x000, .extended = false, .fifo = 0 }
    }};
    
    if (!interface_.configure_filters(filters)) {
        return false;
    }

    // 3. Start Interfejsu (Włącza przerwania / Wątek RX)
    return interface_.init();
}

// --- POLL & LED IMPLEMENTATION ---

void AppCAN::Poll() {
#if defined(PUTM_CAN_BACKEND_STM32)
    // Obsługa LED tylko na mikrokontrolerze
    if (led_config_.enabled) {
        handle_status_led();
    }
#endif
    // Na ROS2 ta metoda nic nie robi (jest pusta), 
    // bo odbiór danych dzieje się w tle w wątku SocketCanHal.
}

#if defined(PUTM_CAN_BACKEND_STM32)
void AppCAN::ConfigStatusLed(GPIO_TypeDef* port, uint16_t pin) {
    led_config_.port = port;
    led_config_.pin = pin;
    led_config_.enabled = true;
}
#endif

void AppCAN::handle_status_led() {
#if defined(PUTM_CAN_BACKEND_STM32)
    auto status = hal_.get_bus_status();
    uint32_t now = HAL_GetTick();
    
    if (status == PUTM_CAN::BusStatus::BUS_OFF) {
        // ERROR: Świeci ciągle
        HAL_GPIO_WritePin(led_config_.port, led_config_.pin, GPIO_PIN_SET);
    }
    else if (status == PUTM_CAN::BusStatus::OK) {
        // OK: Mruga 1Hz (500ms ON / 500ms OFF)
        if ((now % 1000) < 500) {
            HAL_GPIO_WritePin(led_config_.port, led_config_.pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(led_config_.port, led_config_.pin, GPIO_PIN_RESET);
        }
    }
    else {
        // WARNING: Nierówne mruganie
        uint32_t cycle = now % 1000;
        bool state = (cycle < 100) || (cycle > 200 && cycle < 300);
        HAL_GPIO_WritePin(led_config_.port, led_config_.pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
#endif
}