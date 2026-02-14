#ifndef PUTM_EV_CAN_HAL_STM32_HPP
#define PUTM_EV_CAN_HAL_STM32_HPP

#include "can_hal.hpp"
#include "stm32_hal_selector.hpp"
#include <span>

namespace putm_ev_can {

class Stm32CanHal : public PUTM_CAN::ICanHal {
public:
    Stm32CanHal() = default;

    void set_handle(CanHandleType* hcan);

    bool init() override;
    bool transmit(const PUTM_CAN::CanFrame& frame) override;
    bool receive(PUTM_CAN::CanFrame& frame) override; // unused
    bool configure_filters(std::span<const PUTM_CAN::CanFilter> filters) override;
    
    PUTM_CAN::BusDiagnostics get_diagnostics() const override {
        return diagnostics_;
    }

    void set_rx_callback(PUTM_CAN::RxCallback callback) override {
        rx_callback_ = callback;
    }

    void process_irq_read();
    static void process_global_irq(CanHandleType* h);

private:
    CanHandleType* hcan_ = nullptr;
    bool initialized_ = false;
    PUTM_CAN::RxCallback rx_callback_ = nullptr;
    PUTM_CAN::BusDiagnostics diagnostics_; 

    static constexpr size_t MAX_INSTANCES = 3; 
    static Stm32CanHal* registry_[MAX_INSTANCES];
    void register_instance();

    static uint8_t dlc_to_bytes(uint32_t dlc);
    static uint32_t bytes_to_dlc(uint8_t bytes);
};

} // namespace putm_ev_can

#endif