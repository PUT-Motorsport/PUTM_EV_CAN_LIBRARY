/**
 * @file can_hal_stm32.hpp
 * @brief STM32 HAL implementation supporting both FDCAN and bxCAN.
 */

#ifndef PUTM_EV_CAN_HAL_STM32_HPP
#define PUTM_EV_CAN_HAL_STM32_HPP

#include "can_hal.hpp"
#include "stm32_hal_selector.hpp"
#include <span>

namespace putm_ev_can {

/**
 * @brief Adapter for STM32 HAL (FDCAN or bxCAN).
 * @details Implements static registry to dispatch global IRQs to specific C++ instances.
 */
class Stm32CanHal : public PUTM_CAN::ICanHal {
public:
    Stm32CanHal() = default;

    /**
     * @brief Assigns the CAN handle and registers instance in the global registry.
     * @param hcan Pointer to the HAL handle (FDCAN_HandleTypeDef or CAN_HandleTypeDef).
     */
    void set_handle(CanHandleType* hcan);

    bool init() override;
    bool transmit(const PUTM_CAN::CanFrame& frame) override;
    bool receive(PUTM_CAN::CanFrame& frame) override;
    bool configure_filters(std::span<const PUTM_CAN::CanFilter> filters) override;
    PUTM_CAN::BusStatus get_bus_status() const override;

    void set_rx_callback(PUTM_CAN::RxCallback callback) override {
        rx_callback_ = callback;
    }

    bool is_initialized() const override {
        return initialized_ && (hcan_ != nullptr);
    }

    /**
     * @brief Internal method called by the global dispatcher to read FIFO.
     */
    void process_irq_read();

    /**
     * @brief Static dispatcher called from the C-style HAL callback.
     * @param h Handle of the CAN that triggered the interrupt.
     */
    static void process_global_irq(CanHandleType* h);

private:
    CanHandleType* hcan_ = nullptr;
    bool initialized_ = false;
    PUTM_CAN::RxCallback rx_callback_ = nullptr;

    // --- IRQ DISPATCHER MECHANISM ---
    static constexpr size_t MAX_INSTANCES = 3; 
    static Stm32CanHal* registry_[MAX_INSTANCES];
    void register_instance();

    // --- HELPERS ---
    static uint8_t dlc_to_bytes(uint32_t dlc);
    static uint32_t bytes_to_dlc(uint8_t bytes);
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_HAL_STM32_HPP