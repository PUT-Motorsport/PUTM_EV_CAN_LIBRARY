#ifndef CAN_HAL_HPP
#define CAN_HAL_HPP

#include <cstdint>
#include <array>

namespace PUTM_CAN {

/**
 * @brief Structure representing a CAN frame.
 * 
 * Contains the ID, data length code (DLC), and a fixed-size data buffer of 8 bytes.
 */
struct CanFrame {
    uint32_t id;                        /**< CAN message ID (11-bit or 29-bit). */
    uint8_t dlc;                        /**< Data length code (0-8 bytes). */
    std::array<uint8_t, 8> data = {};   /**< Data buffer for the CAN frame. */
};

/**
 * @brief Pure virtual interface for CAN hardware abstraction.
 * 
 * Defines the basic operations for CAN interface, to be implemented by derived classes.
 */
class ICanHal {
public:
    virtual ~ICanHal() = default;       /**< Virtual destructor for proper cleanup. */

    /**
     * @brief Initializes the CAN interface.
     * @return true if initialization succeeded, false otherwise.
     */
    virtual bool init() = 0;

    /**
     * @brief Transmits a CAN frame.
     * @param frame The CAN frame to send.
     * @return true if transmission succeeded, false otherwise.
     */
    virtual bool transmit(const CanFrame& frame) = 0;

    /**
     * @brief Receives a CAN frame (non-blocking).
     * 
     * Returns true if a new message was received, false otherwise.
     * @param frame Reference to the CAN frame to fill.
     * @return true if a message was received, false otherwise.
     */
    virtual bool receive(CanFrame& frame) = 0;
};

} // namespace PUTM_CAN

#endif