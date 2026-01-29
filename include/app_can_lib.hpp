/**
 * @file app_can_lib.hpp
 * @brief User-friendly Facade for the CAN library (STM32 Cross-Family Compatible).
 * @author PUT Motorsport
 */

#ifndef APP_CAN_LIB_HPP
#define APP_CAN_LIB_HPP

#include "PUTM_EV_CAN_LIBRARY/include/stm32_hal_selector.hpp"
#include "PUTM_CAN_M.h"
#include <functional>
#include <array>

// Library components
#include "PUTM_EV_CAN_LIBRARY/include/message_handler.hpp"
#include "PUTM_EV_CAN_LIBRARY/include/can_hal_stm32.hpp"
#include "PUTM_EV_CAN_LIBRARY/include/can_interface.hpp"

/**
 * @brief Class representing a single physical CAN interface.
 * @note Create one instance per CAN peripheral.
 */
class AppCAN {
public:
    AppCAN() : interface_(hal_, handler_) {}
    
    // Disable copying
    AppCAN(const AppCAN&) = delete;
    AppCAN& operator=(const AppCAN&) = delete;

    /**
     * @brief Initializes this CAN instance.
     * @param hcan Pointer to the HAL handle (e.g. &hfdcan1 for G4, &hcan1 for F4).
     * @return true on success.
     */
    bool Init(putm_ev_can::CanHandleType* hcan);

    /**
     * @brief Polls for updates (mainly for Diagnostic LED).
     * @details RX is handled by interrupts. Call this in main loop.
     */
    void Poll();

    /**
     * @brief Configures a GPIO pin to act as a status indicator.
     */
    void ConfigStatusLed(GPIO_TypeDef* port, uint16_t pin) {
        led_config_.port = port;
        led_config_.pin = pin;
        led_config_.enabled = true;
    }

    /**
     * @brief Sends a CAN frame.
     */
    template <typename MsgType>
    bool Send(uint32_t id, const MsgType& msg) {
        return interface_.send(id, msg);
    }

    /**
     * @brief Registers a callback for a specific frame ID.
     */
    template <typename MsgType>
    void RegisterCallback(uint32_t id, std::function<void(const MsgType&)> callback) {
        handler_.register_callback<MsgType>(id, callback);
    }

private:
    struct StatusLedConfig {
        GPIO_TypeDef* port = nullptr;
        uint16_t pin = 0;
        bool enabled = false;
    };

    void handle_status_led();

    putm_ev_can::Stm32CanHal hal_;
    putm_ev_can::MessageHandler handler_;
    putm_ev_can::DefaultCanInterface interface_;
    
    StatusLedConfig led_config_;
};

#endif // APP_CAN_LIB_HPP