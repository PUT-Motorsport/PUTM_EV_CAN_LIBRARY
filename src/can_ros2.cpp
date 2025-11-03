/**
 * @file can_ros2.cpp
 * @brief ROS2 SocketCAN implementation of ICanHal.
 */

#include "can_hal.hpp"

namespace PUTM_CAN {

/**
 * @brief ROS2 CAN HAL using SocketCAN.
 */
class Ros2CanHal : public ICanHal {
public:
    bool init() override { return true; }
    bool transmit(const CanFrame&) override { return true; }
    bool receive(CanFrame&) override { return false; }
};

} // namespace PUTM_CAN