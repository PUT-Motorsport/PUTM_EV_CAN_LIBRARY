/**
 * @file can_hal_ros2.hpp
 * @brief SocketCAN implementation for ROS2 environment.
 */

#ifndef PUTM_EV_CAN_HAL_ROS2_HPP
#define PUTM_EV_CAN_HAL_ROS2_HPP

#include "can_hal.hpp"
#include <string>
#include <thread>
#include <atomic>

namespace putm_ev_can {

/**
 * @brief Uses Linux SocketCAN to emulate hardware behavior on PC.
 */
class SocketCanHal : public PUTM_CAN::ICanHal {
public:
    SocketCanHal() = default;
    ~SocketCanHal();

    /**
     * @brief Initializes the socket on a specific interface.
     * @param interface_name e.g. "can0", "vcan0"
     */
    bool init(const std::string& interface_name);

    // Overrides from ICanHal
    bool init() override { return false; } // Not used directly
    bool transmit(const PUTM_CAN::CanFrame& frame) override;
    bool receive(PUTM_CAN::CanFrame& frame) override;
    
    void set_rx_callback(PUTM_CAN::RxCallback callback) override {
        rx_callback_ = callback;
    }

    bool is_initialized() const override { return connected_; }
    PUTM_CAN::BusStatus get_bus_status() const override { return PUTM_CAN::BusStatus::OK; }

private:
    int socket_fd_ = -1;
    bool connected_ = false;
    PUTM_CAN::RxCallback rx_callback_ = nullptr;

    // Thread management for RX loop
    std::atomic<bool> running_{false};
    std::thread rx_thread_;

    void rx_thread_loop();
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_HAL_ROS2_HPP