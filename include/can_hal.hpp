/**
 * @file can_hal.hpp
 * @brief Hardware Abstraction Layer for CAN (common for STM32/ROS2).
 */

#ifndef CAN_HAL_HPP
#define CAN_HAL_HPP

#include <cstdint>
#include <array>
#include <vector>

#include "Inc/fdcan.h"

namespace PUTM_CAN {

/**
 * @brief Raw CAN frame structure.
 */
struct CanFrame {
    uint32_t id = 0;                    ///< CAN ID (11-bit or 29-bit)
    uint8_t  dlc = 0;                   ///< Data Length Code (0–8)
    std::array<uint8_t, 8> data{};      ///< Payload buffer
};

/**
 * @brief Simple filter description (mask-based).
 */
struct CanFilter {
    uint32_t id = 0;        ///< Match ID
    uint32_t mask = 0x7FF;  ///< Mask (11-bit default)
    bool extended = false;  ///< false=standard(11b), true=extended(29b)
    uint8_t fifo = 0;       ///< Target FIFO (STM32: 0/1), optional elsewhere
};

/**
 * @brief Pure virtual HAL interface (platform-specific implementations).
 */
class ICanHal {
public:
    virtual ~ICanHal() = default;

    /// Initialize CAN peripheral/bus.
    virtual bool init() = 0;

    /// Transmit one frame.
    virtual bool transmit(const CanFrame& frame) = 0;

    /// Receive one frame (non-blocking). Return true if a frame was read.
    virtual bool receive(CanFrame& frame) = 0;

    /// Optional: configure hardware filters (soft fallback in higher layer).
    virtual bool configure_filters(const std::vector<CanFilter>&) { return true; }

    /// Optional: query ready/initialized state.
    virtual bool is_initialized() const { return true; }
};

} // namespace PUTM_CAN

#endif // CAN_HAL_HPP
