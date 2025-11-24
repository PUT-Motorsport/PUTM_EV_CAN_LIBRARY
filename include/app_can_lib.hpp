#ifndef APP_CAN_LIB_HPP
#define APP_CAN_LIB_HPP

#include "stm32g4xx_hal.h"
#include "PUTM_CAN_1.h"
#include <functional>

// Dołączamy komponenty wewnętrzne biblioteki
#include "PUTM_EV_CAN_LIBRARY/include/message_handler.hpp"
#include "PUTM_EV_CAN_LIBRARY/include/can_hal_stm32.hpp"
#include "PUTM_EV_CAN_LIBRARY/include/can_interface.hpp"

// Klasa AppCAN - każda instancja to osobna magistrala (np. Bus1, Bus2)
class AppCAN {
public:
    AppCAN() = default;
    
    // Usuwamy konstruktor kopiujący, żeby uniknąć problemów z podwójnym zwalnianiem pamięci
    AppCAN(const AppCAN&) = delete;
    AppCAN& operator=(const AppCAN&) = delete;

    /**
     * @brief Inicjalizuje tę konkretną instancję CAN.
     * @param hfdcan Uchwyt z CubeMX (np. &hfdcan1)
     */
    bool Init(FDCAN_HandleTypeDef* hfdcan);

    /**
     * @brief Sprawdza czy przyszły dane na TYM interfejsie.
     */
    void Poll();

    /**
     * @brief Wysyła ramkę przez TEN interfejs.
     */
    template <typename MsgType>
    bool Send(uint32_t id, const MsgType& msg) {
        if (!interface_) return false;
        return interface_->send(id, msg);
    }

    /**
     * @brief Rejestruje callback dla TEGO interfejsu.
     */
    template <typename MsgType>
    void RegisterCallback(uint32_t id, std::function<void(const MsgType&)> callback) {
        handler_.register_callback<MsgType>(id, callback);
    }

private:
    // Każda instancja ma swój własny zestaw obiektów:
    putm_ev_can::MessageHandler handler_;
    putm_ev_can::Stm32CanHal* hal_ = nullptr;
    putm_ev_can::CanInterface* interface_ = nullptr;
};

#endif // APP_CAN_LIB_HPP