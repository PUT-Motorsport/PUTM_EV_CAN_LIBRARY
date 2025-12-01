/**
 * @file can_ros2.cpp
 * @brief Minimal SocketCAN/ROS2 HAL stub.
 * @details This file serves as a placeholder for non-embedded (Linux/ROS2) implementations.
 * It has been updated to match the embedded-friendly interface (std::span).
 */

#include "PUTM_EV_CAN_LIBRARY/include/can_hal.hpp"

namespace putm_ev_can {

/**
 * @brief Stub implementation of HAL for ROS2/Linux systems.
 */
class Ros2CanHal : public PUTM_CAN::ICanHal {
public:
    bool init() override { initialized_ = true; return true; }

    bool transmit(const PUTM_CAN::CanFrame& /*frame*/) override {
        // TODO: Implement SocketCAN send here
        return true;
    }

    bool receive(PUTM_CAN::CanFrame& /*frame*/) override {
        // TODO: Implement SocketCAN receive here (non-blocking)
        return false;
    }

    // FIX: Updated signature to match ICanHal (std::span instead of std::vector)
    bool configure_filters(std::span<const PUTM_CAN::CanFilter> /*filters*/) override {
        // TODO: Implement setsockopt(CAN_RAW_FILTER) here.
        return true;
    }

    bool is_initialized() const override { return initialized_; }

private:
    bool initialized_ = false;
};

} // namespace putm_ev_can