/**
 * @file app_can_lib.hpp
 * @brief User-friendly Facade for the CAN library.
 * @details Supports multiple CAN instances and avoids dynamic allocation.
 * @author PUT Motorsport
 */

#ifndef APP_CAN_LIB_HPP
#define APP_CAN_LIB_HPP

#include "stm32g4xx_hal.h"
#include "PUTM_CAN_M.h"
#include <functional>
#include <array>

// Library components
#include "PUTM_EV_CAN_LIBRARY/include/message_handler.hpp"
#include "PUTM_EV_CAN_LIBRARY/include/can_hal_stm32.hpp"
#include "PUTM_EV_CAN_LIBRARY/include/can_interface.hpp"

/**
 * @brief Class representing a single physical CAN interface.
 * @note Create one instance per FDCAN peripheral.
 */
class AppCAN {
public:
    /**
     * @brief Constructor initializing internal components via composition.
     */
    AppCAN() : interface_(hal_, handler_) {}

    // Disable copying
    AppCAN(const AppCAN&) = delete;
    AppCAN& operator=(const AppCAN&) = delete;

    /**
     * @brief Initializes this CAN instance.
     * @param hfdcan Pointer to the CubeMX handle (e.g., &hfdcan1).
     * @return true on success.
     */
    bool Init(FDCAN_HandleTypeDef* hfdcan);

    /**
     * @brief Polls for new messages and updates the status LED.
     * @details Call this cyclically in the main loop.
     */
    void Poll();

    /**
     * @brief Configures a GPIO pin to act as a status indicator.
     * @details LED behavior:
     * - Steady ON: Bus Off (Error)
     * - 1Hz Blink: OK
     * - Irregular Blink: Warning (Bus Full/Error Passive)
     * * @param port GPIO Port (e.g., GPIOA)
     * @param pin GPIO Pin (e.g., GPIO_PIN_5)
     */
    void ConfigStatusLed(GPIO_TypeDef* port, uint16_t pin) {
        led_config_.port = port;
        led_config_.pin = pin;
        led_config_.enabled = true;
    }

    /**
     * @brief Sends a CAN frame.
     * @tparam MsgType Message structure type.
     * @param id Frame ID.
     * @param msg Data structure.
     * @return true if added to TX queue.
     */
    template <typename MsgType>
    bool Send(uint32_t id, const MsgType& msg) {
        return interface_.send(id, msg);
    }

    /**
     * @brief Registers a callback for a specific frame ID.
     * @param id Frame ID to listen for.
     * @param callback Function to handle the message.
     */
    template <typename MsgType>
    void RegisterCallback(uint32_t id, std::function<void(const MsgType&)> callback) {
        handler_.register_callback<MsgType>(id, callback);
    }

private:
    /**
     * @brief Internal configuration for the status LED.
     */
    struct StatusLedConfig {
        GPIO_TypeDef* port = nullptr; ///< GPIO Port
        uint16_t pin = 0;             ///< GPIO Pin
        bool enabled = false;         ///< Is LED configured?
    };

    /**
     * @brief Updates the status LED based on bus state.
     */
    void handle_status_led();

    putm_ev_can::Stm32CanHal hal_;
    putm_ev_can::MessageHandler handler_;
    putm_ev_can::DefaultCanInterface interface_;
    
    StatusLedConfig led_config_;
};

#endif // APP_CAN_LIB_HPP