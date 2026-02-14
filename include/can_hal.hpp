/**
 * @file can_hal.hpp
 * @brief Abstract Hardware Abstraction Layer.
 */

#ifndef CAN_HAL_HPP
#define CAN_HAL_HPP

#include <cstdint>
#include <array>
#include <span> 
#include <functional>

namespace PUTM_CAN {

using CanId = uint32_t;

/**
 * @brief Diagnostics statistics.
 */
struct BusDiagnostics {
    uint32_t rx_count = 0;
    uint32_t tx_count = 0;
    uint32_t tx_errors = 0;
    uint32_t rx_errors = 0;
    uint32_t last_error_code = 0;
};

/**
 * @brief Universal CAN Frame (Classic + FD).
 * @note Payload increased to 64 bytes for CAN FD support.
 */
struct CanFrame {
    CanId    id = 0;
    uint8_t  dlc = 0; 
    std::array<uint8_t, 64> data{}; 
    bool     is_fd = false;
};

using RxCallback = std::function<void(const CanFrame&)>;

struct CanFilter {
    CanId    id = 0;
    uint32_t mask = 0x7FF;
    bool     extended = false;
    uint8_t  fifo = 0;
};

class ICanHal {
public:
    virtual ~ICanHal() = default;

    virtual bool init() { return false; }
    virtual bool transmit(const CanFrame& frame) { return false; }
    virtual bool receive(CanFrame& frame) { return false; }
    
    virtual bool configure_filters(std::span<const CanFilter> filters) { return false; }
    virtual void set_rx_callback(RxCallback callback) {}
    
    virtual BusDiagnostics get_diagnostics() const { return {}; }
};

} // namespace PUTM_CAN

#endif