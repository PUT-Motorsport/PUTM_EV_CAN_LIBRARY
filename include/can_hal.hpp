/**
 * @file can_hal.hpp
 * @brief Hardware Abstraction Layer (HAL) for CAN bus.
 * @details Contains definitions for raw frames, filters, and the abstract interface.
 * @author PUT Motorsport
 * @date 2025
 */

#ifndef CAN_HAL_HPP
#define CAN_HAL_HPP

#include <cstdint>
#include <array>
#include <span> 

namespace PUTM_CAN {

/**
 * @brief Global definition of the CAN Identifier type.
 */
using CanId = uint32_t;

/**
 * @brief Represents the current state of the CAN bus.
 * @note Added for LED status indication.
 */
enum class BusStatus {
    OK,      ///< Bus is operating normally (Error Active).
    WARNING, ///< Bus is in Warning or Error Passive state (transient errors).
    BUS_OFF  ///< Bus is Off (critical error, no communication).
};

/**
 * @brief Structure representing a raw CAN frame.
 */
struct CanFrame {
    CanId    id = 0;               ///< Frame Identifier (Standard 11-bit or Extended 29-bit)
    uint8_t  dlc = 0;              ///< Data Length Code (0-8)
    std::array<uint8_t, 8> data{}; ///< Data payload buffer (initialized to zeros)
};

/**
 * @brief Simple mask-based CAN filter structure.
 */
struct CanFilter {
    CanId    id = 0;        ///< ID to match
    uint32_t mask = 0x7FF;  ///< Bit mask (default 0x7FF for standard frames)
    bool     extended = false;  ///< false = Standard (11b), true = Extended (29b)
    uint8_t  fifo = 0;       ///< Target FIFO (for STM32: 0 or 1)
};

/**
 * @brief Abstract HAL interface.
 * @details Inheriting classes must implement init, transmit, and receive methods.
 */
class ICanHal {
public:
    virtual ~ICanHal() = default;

    /**
     * @brief Initializes the hardware peripheral.
     * @return true on success, false on error.
     */
    virtual bool init() = 0;

    /**
     * @brief Transmits a CAN frame.
     * @param frame The frame to be sent.
     * @return true if the frame was successfully added to the TX queue, false otherwise.
     */
    virtual bool transmit(const CanFrame& frame) = 0;

    /**
     * @brief Receives a CAN frame (non-blocking).
     * @param[out] frame Reference to a structure where received data will be stored.
     * @return true if a new frame was read, false if no data available.
     */
    virtual bool receive(CanFrame& frame) = 0;
    
    /**
     * @brief Configures hardware filters.
     * @param filters Span of filter structures to be applied.
     * @return true if configuration was successful.
     */
    virtual bool configure_filters(std::span<const CanFilter> filters) { return true; }

    /**
     * @brief Checks if the hardware is initialized.
     */
    virtual bool is_initialized() const { return true; }

    /**
     * @brief Retrieves the current diagnostic status of the bus.
     * @return Current BusStatus (OK, WARNING, or BUS_OFF).
     * @note Default implementation returns OK to satisfy interface requirements if not implemented.
     */
    virtual BusStatus get_bus_status() const { return BusStatus::OK; }
};

} // namespace PUTM_CAN

#endif // CAN_HAL_HPP