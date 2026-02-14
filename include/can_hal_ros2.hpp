#ifndef PUTM_EV_CAN_HAL_ROS2_HPP
#define PUTM_EV_CAN_HAL_ROS2_HPP

#include "can_hal.hpp"
#include <string>
#include <thread>
#include <atomic>

namespace putm_ev_can {

class SocketCanHal : public PUTM_CAN::ICanHal {
public:
    SocketCanHal() = default;
    ~SocketCanHal();

    bool init(const std::string& interface_name);

    // Overrides
    bool init() override { return false; } 
    bool transmit(const PUTM_CAN::CanFrame& frame) override;
    bool receive(PUTM_CAN::CanFrame& frame) override;
    
    void set_rx_callback(PUTM_CAN::RxCallback callback) override {
        rx_callback_ = callback;
    }

    bool is_initialized() const override { return connected_; }
    PUTM_CAN::BusDiagnostics get_diagnostics() const override { return diagnostics_; }

private:
    int socket_fd_ = -1;
    bool connected_ = false;
    PUTM_CAN::RxCallback rx_callback_ = nullptr;
    PUTM_CAN::BusDiagnostics diagnostics_;

    std::atomic<bool> running_{false};
    std::thread rx_thread_;

    void rx_thread_loop();
};

} // namespace putm_ev_can

#endif