/**
 * @file can_hal_stm32.hpp
 * @brief STM32G4 FDCAN HAL implementation with Interrupt Support.
 */

#ifndef PUTM_EV_CAN_HAL_STM32_HPP
#define PUTM_EV_CAN_HAL_STM32_HPP

#include "can_hal.hpp"
#include "stm32g4xx_hal.h" 
#include <span>

namespace putm_ev_can {

/**
 * @brief Adapter for STM32 HAL (FDCAN).
 * @details Implements static registry to dispatch global IRQs to specific C++ instances.
 */
class Stm32CanHal : public PUTM_CAN::ICanHal {
public:
    Stm32CanHal() = default;

    /**
     * @brief Assigns the FDCAN handle and registers instance in the global registry.
     * @param hfdcan Pointer to the handle from CubeMX.
     */
    void set_handle(FDCAN_HandleTypeDef* hfdcan);

    bool init() override;
    bool transmit(const PUTM_CAN::CanFrame& frame) override;
    bool receive(PUTM_CAN::CanFrame& frame) override;
    bool configure_filters(std::span<const PUTM_CAN::CanFilter> filters) override;
    PUTM_CAN::BusStatus get_bus_status() const override;

    /**
     * @brief Registers the ISR callback.
     */
    void set_rx_callback(PUTM_CAN::RxCallback callback) override {
        rx_callback_ = callback;
    }

    bool is_initialized() const override {
        return initialized_ && (hfdcan_ != nullptr);
    }

    /**
     * @brief Internal method called by the global dispatcher to read FIFO.
     */
    void process_irq_read();

    /**
     * @brief Static dispatcher called from the C-style HAL callback.
     * @param h Handle of the FDCAN that triggered the interrupt.
     */
    static void process_global_irq(FDCAN_HandleTypeDef* h);

private:
    static uint8_t dlc_to_bytes(uint32_t fdcan_dlc);
    
    /**
     * @brief Registers this instance in the static array.
     */
    void register_instance();

    FDCAN_HandleTypeDef* hfdcan_ = nullptr;
    bool initialized_ = false;
    PUTM_CAN::RxCallback rx_callback_ = nullptr;

    // --- IRQ DISPATCHER MECHANISM ---
    static constexpr size_t MAX_INSTANCES = 3; 
    static Stm32CanHal* registry_[MAX_INSTANCES];
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_HAL_STM32_HPP