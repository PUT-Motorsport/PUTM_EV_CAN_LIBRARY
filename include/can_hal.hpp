/**
 * @file can_hal.hpp
 * @brief Hardware Abstraction Layer for CAN.
 *
 * Defines raw CAN frame and pure virtual interface.
 */

#ifndef CAN_HAL_HPP
#define CAN_HAL_HPP

#include <cstdint>
#include <array>

namespace PUTM_CAN {

/**
 * @brief Raw CAN frame structure.
 */
struct CanFrame {
    uint32_t id;                        ///< CAN ID (11-bit or 29-bit)
    uint8_t dlc;                        ///< Data Length Code (0–8)
    std::array<uint8_t, 8> data = {};   ///< Payload buffer
};

/**
 * @brief Pure virtual HAL interface.
 */
class ICanHal {
public:
    virtual ~ICanHal() = default;

    /** @brief Initialize CAN peripheral */
    virtual bool init() = 0;

    /** @brief Transmit a frame */
    virtual bool transmit(const CanFrame& frame) = 0;

    /** @brief Receive a frame (non-blocking) */
    virtual bool receive(CanFrame& frame) = 0;
};

} // namespace PUTM_CAN

#endif // CAN_HAL_HPP