#include "app_can_lib.hpp"
#include <vector>

bool AppCAN::Init(FDCAN_HandleTypeDef* hfdcan) {
    if (!hfdcan) return false;

    // 1. Ustawienie blokady przerwań (wspólne dla wszystkich, ale ustawiane per instancja)
    handler_.set_locking_mechanism(
        []() { __disable_irq(); },
        []() { __enable_irq(); }
    );

    // 2. Tworzymy nowy obiekt HAL dedykowany dla tego uchwytu (hfdcan1 lub hfdcan2)
    hal_ = new putm_ev_can::Stm32CanHal(hfdcan);

    // 3. Tworzymy główny interfejs
    interface_ = putm_ev_can::make_default_interface(hal_, handler_);

    if (!interface_) return false;

    // 4. Konfigurujemy filtry (domyślnie przepuść wszystko)
    std::vector<PUTM_CAN::CanFilter> filters;
    filters.push_back({
        .id = 0,
        .mask = 0x000, // Maska 0 = Don't care -> odbierz wszystko
        .extended = false,
        .fifo = 0
    });
    
    interface_->configure_filters(filters);

    // 5. Uruchamiamy ten konkretny interfejs
    return interface_->init();
}

void AppCAN::Poll() {
    if (interface_) {
        interface_->process_received_messages();
    }
}