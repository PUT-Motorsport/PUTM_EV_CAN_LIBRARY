/**
 * @file app_can_lib.hpp
 * @brief User-friendly Facade for the CAN library (STM32 & ROS2 Compatible).
 */

#ifndef APP_CAN_LIB_HPP
#define APP_CAN_LIB_HPP

#include "PUTM_CAN_M.h"
#include <functional>
#include <array>

// --- BACKEND SELECTION ---
#if defined(PUTM_CAN_BACKEND_STM32)
    #include "PUTM_EV_CAN_LIBRARY/include/can_hal_stm32.hpp"
    // Alias dla typu argumentu Init na STM32 (wskaźnik do uchwytu)
    using HalType = putm_ev_can::Stm32CanHal;
    using InitParamType = putm_ev_can::CanHandleType*; 

#elif defined(PUTM_CAN_BACKEND_ROS2)
    #include "PUTM_EV_CAN_LIBRARY/include/can_hal_ros2.hpp"
    #include <string>
    // Alias dla typu argumentu Init na Linux/ROS (nazwa interfejsu np. "can0")
    using HalType = putm_ev_can::SocketCanHal;
    using InitParamType = const std::string&;

#else
    #error "PUTM CAN LIB: No backend defined! Use -DPUTM_CAN_BACKEND=STM32 or ROS2"
#endif

// Library components
#include "PUTM_EV_CAN_LIBRARY/include/message_handler.hpp"
#include "PUTM_EV_CAN_LIBRARY/include/can_interface.hpp"

class AppCAN {
public:
    AppCAN() : interface_(hal_, handler_) {}
    
    AppCAN(const AppCAN&) = delete;
    AppCAN& operator=(const AppCAN&) = delete;

    /**
     * @brief Initializes CAN interface.
     * @param param STM32: &hfdcan1 | Linux: "can0"
     */
    bool Init(InitParamType param);

    /**
     * @brief Main loop poll. Handles LED on STM32. Empty on Linux.
     */
    void Poll();

    // Funkcje diagnostyczne dostępne TYLKO na STM32
#if defined(PUTM_CAN_BACKEND_STM32)
    void ConfigStatusLed(GPIO_TypeDef* port, uint16_t pin);
#endif

    template <typename MsgType>
    bool Send(uint32_t id, const MsgType& msg) {
        return interface_.send(id, msg);
    }

    template <typename MsgType>
    void RegisterCallback(uint32_t id, std::function<void(const MsgType&)> callback) {
        handler_.register_callback<MsgType>(id, callback);
    }

private:
    void handle_status_led();

    HalType hal_;
    putm_ev_can::MessageHandler handler_;
    putm_ev_can::DefaultCanInterface interface_;
    
    struct StatusLedConfig {
#if defined(PUTM_CAN_BACKEND_STM32)
        GPIO_TypeDef* port = nullptr;
#endif
        uint16_t pin = 0;
        bool enabled = false;
    } led_config_;
};

#endif // APP_CAN_LIB_HPP