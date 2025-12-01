/**
 * @file can_hal_stm32.hpp
 * @brief STM32G4 FDCAN HAL implementation.
 */

#ifndef PUTM_EV_CAN_HAL_STM32_HPP
#define PUTM_EV_CAN_HAL_STM32_HPP

#include "can_hal.hpp"
#include "stm32g4xx_hal.h" 
#include <span>

namespace putm_ev_can {

/**
 * @brief Adapter for STM32 HAL (FDCAN).
 */
class Stm32CanHal : public PUTM_CAN::ICanHal {
public:
    Stm32CanHal() = default;

    /**
     * @brief Assigns the FDCAN handle.
     * @param hfdcan Pointer to the handle from CubeMX.
     */
    void set_handle(FDCAN_HandleTypeDef* hfdcan) { hfdcan_ = hfdcan; }

    bool init() override;
    bool transmit(const PUTM_CAN::CanFrame& frame) override;
    bool receive(PUTM_CAN::CanFrame& frame) override;
    bool configure_filters(std::span<const PUTM_CAN::CanFilter> filters) override;
    
    bool is_initialized() const override {
        return initialized_ && (hfdcan_ != nullptr);
    }

private:
    static uint8_t dlc_to_bytes(uint32_t fdcan_dlc);

    FDCAN_HandleTypeDef* hfdcan_ = nullptr;
    bool initialized_ = false;
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_HAL_STM32_HPP