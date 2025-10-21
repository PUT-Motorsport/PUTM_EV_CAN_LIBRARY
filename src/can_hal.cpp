#ifndef CAN_HAL_HPP
#define CAN_HAL_HPP

#include <cstdint>
#include <array>

namespace PUTM_CAN {

// Definicja surowej ramki CAN
struct CanFrame {
    uint32_t id;                        // 11-bitowy lub 29-bitowy ID
    uint8_t dlc;                        // Data Length Code (0-8)
    std::array<uint8_t, 8> data = {};   // Maksymalnie 8 bajtów danych
};

// Czysty interfejs abstrakcji sprzętowej
class ICanHal {
public:
    virtual ~ICanHal() = default;

    // Inicjalizuje interfejs CAN
    virtual bool init() = 0;

    // Wysyła ramkę CAN
    virtual bool transmit(const CanFrame& frame) = 0;

    // Odbiera ramkę CAN (nieblokujące)
    // Zwraca true, jeśli odebrano nową wiadomość
    virtual bool receive(CanFrame& frame) = 0;
};

} // namespace PUTM_CAN

#endif