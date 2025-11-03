/**
 * @file can_hal.cpp
 * @brief Optional default implementation of ICanHal for simulation.
 *
 * This file provides a stub implementation of ICanHal that can be used
 * for unit testing or simulation without hardware. It does not transmit
 * or receive real CAN frames.
 *
 * @note This is a simulation-only HAL. For real hardware, use Stm32CanHal
 *       or Ros2CanHal.
 */

#include "can_hal.hpp"
#include <cstring>

namespace PUTM_CAN {

/**
 * @brief Default simulation HAL.
 *
 * Always reports success for transmit, never has data to receive.
 */
class DefaultCanHal : public ICanHal {
public:
    /**
     * @brief Initialize simulation HAL.
     * @return Always true
     */
    bool init() override {
        return true;
    }

    /**
     * @brief Simulate transmission.
     * @param frame Frame to "send"
     * @return Always true (simulated success)
     */
    bool transmit(const CanFrame& frame) override {
        (void)frame;  // Suppress unused warning
        return true;
    }

    /**
     * @brief Simulate reception.
     * @param[out] frame Frame to fill
     * @return Always false (no data)
     */
    bool receive(CanFrame& frame) override {
        std::memset(&frame, 0, sizeof(frame));
        return false;
    }
};

/**
 * @brief Factory function to create default HAL.
 * @return Pointer to simulation HAL
 *
 * @warning Caller must manage lifetime (delete when done).
 */
ICanHal* create_default_hal() {
    return new DefaultCanHal();
}

} // namespace PUTM_CAN