/**
 * @file can_driver.hpp
 * @brief Main API class. Replaces the old AppCAN.
 */

#ifndef PUTM_EV_CAN_DRIVER_HPP
#define PUTM_EV_CAN_DRIVER_HPP

#include "can_hal.hpp"
#include "message_handler.hpp"
#include <concepts>
#include <cstring> 

// --- PLATFORM SELECTION ---
#if defined(PUTM_CAN_BACKEND_STM32)
    #include "can_hal_stm32.hpp"
    using HalType = putm_ev_can::Stm32CanHal;
    using InitParamType = putm_ev_can::CanHandleType*; 
#elif defined(PUTM_CAN_BACKEND_ROS2)
    #include "can_hal_ros2.hpp"
    #include <string>
    using HalType = putm_ev_can::SocketCanHal;
    using InitParamType = const std::string&;
#else
    #error "Backend not defined! Check CMake configuration."
#endif

namespace putm_ev_can {

// C++20 Concept
template<typename T>
concept CanMessage = requires(T a) {
    sizeof(T) > 0;
};

class CanDriver {
public:
    CanDriver() = default;
    ~CanDriver(); 

    // No copy
    CanDriver(const CanDriver&) = delete;
    CanDriver& operator=(const CanDriver&) = delete;

    bool Init(InitParamType param);

#if defined(PUTM_CAN_BACKEND_ROS2)
    /**
     * @brief Initializes the bus with explicit configuration (bitrate, FD, ...).
     * @code
     *   putm_ev_can::SocketCanConfig cfg;
     *   cfg.interface = "can0";
     *   cfg.bitrate   = 1'000'000;
     *   can_m.Init(cfg);
     * @endcode
     */
    bool Init(const SocketCanConfig& config);
#endif

    /// Stops reception. Safe to call multiple times; also called by the destructor.
    void Stop();

    PUTM_CAN::BusDiagnostics GetDiagnostics() const;

    /**
     * @brief Sends a CAN message using DBC packing.
     */
    template <CanMessage MsgType>
    bool Send(PUTM_CAN::CanId id, const MsgType& msg) {
        const auto* e = find_dbc_entry(id);
        if (!e) return false; 

        uint8_t buffer[64] = {0};
        
        using PackFn = int(*)(uint8_t*, const void*, size_t);
        auto pack_fn = reinterpret_cast<PackFn>(e->pack);

        if (pack_fn(buffer, &msg, e->len) < 0) {
            return false; 
        }

        PUTM_CAN::CanFrame frame{};
        frame.id = id;
        frame.dlc = e->len;
        frame.is_fd = (e->len > 8);
        std::memcpy(frame.data.data(), buffer, frame.dlc);
        
        return hal_.transmit(frame);
    }

    /**
     * @brief Registers a RX callback.
     * @warning STM32: executed in INTERRUPT CONTEXT! Keep it short.
     * @warning ROS2: executed in the RX thread of THIS bus. Two buses = two threads running
     *          callbacks concurrently -> protect data shared between buses. A slow callback
     *          delays reception of this bus (frames counted in diagnostics.rx_dropped).
     */
    template <CanMessage MsgType>
    void RegisterCallback(PUTM_CAN::CanId id, std::function<void(const MsgType&)> callback) {
        handler_.register_callback<MsgType>(id, callback);
    }

private:
    bool finish_init();

    // Order matters: members are destroyed in reverse order, and hal_ (RX thread)
    // calls into handler_. The destructor stops hal_ explicitly first anyway.
    MessageHandler handler_;
    HalType hal_;
};

} // namespace putm_ev_can

#endif