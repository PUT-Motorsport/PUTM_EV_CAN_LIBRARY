#ifndef PUTM_EV_CAN_HAL_STM32_HPP
#define PUTM_EV_CAN_HAL_STM32_HPP

#include "can_hal.hpp"
#include "stm32g4xx_hal.h" 

namespace putm_ev_can {

class Stm32CanHal : public PUTM_CAN::ICanHal {
public:
    // Konstruktor przyjmuje wskaźnik do uchwytu (np. &hfdcan1)
    explicit Stm32CanHal(FDCAN_HandleTypeDef* hfdcan);

    bool init() override;
    bool transmit(const PUTM_CAN::CanFrame& frame) override;
    bool receive(PUTM_CAN::CanFrame& frame) override;
    bool configure_filters(const std::vector<PUTM_CAN::CanFilter>& filters) override;
    bool is_initialized() const override;

private:
    static uint8_t dlc_to_bytes(uint32_t fdcan_dlc);

    FDCAN_HandleTypeDef* hfdcan_ = nullptr;
    bool initialized_ = false;
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_HAL_STM32_HPP