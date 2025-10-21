#include "can_interface.hpp"
#include "can_hal.hpp"
#include "generated/can_definitions.pb.h" // Generowany przez protoc

namespace PUTM_CAN {

// Definicja mapowania ID na typ Protobuf (może być generowana skryptem)
// W rzeczywistości użyjesz własnego generatora, aby automatycznie to stworzyć.
struct CanMessageMetadata {
    uint32_t id;
    putm_can::PcMainData pc_main_data; // Przykład buforowania w klasie CanInterface
};

CanInterface::CanInterface(ICanHal* hal_ptr) : hal(hal_ptr) {
    // Inicjalizacja HAL
    hal->init();
    // Inicjalizacja buforów wiadomości, jeśli potrzebne
}

void CanInterface::handle_incoming_messages() {
    CanFrame frame;
    // Odbieraj wszystkie dostępne ramki
    while (hal->receive(frame)) {
        switch (frame.id) {
            case 0x60: // PC_TEMPERATURE_CAN_ID
                // 1. Deserializacja Protobuf
                // Sprawdź rozmiar, aby uniknąć przepełnienia:
                if (frame.dlc <= frame.data.size()) {
                    // Wykorzystanie wbudowanej funkcji deserializacji Protobuf
                    current_pc_temp_data.ParseFromArray(frame.data.data(), frame.dlc);
                    pc_temperature_new_data = true;
                }
                break;
            // ... inne przypadki dla 0x10, 0x57, itd.
            default:
                // Nieznane ID
                break;
        }
    }
}

template <typename T>
bool CanInterface::send_message(const T& msg, uint32_t can_id) {
    CanFrame frame;
    frame.id = can_id;

    // 1. Serializacja Protobuf
    // Wymagany bufor, ponieważ SerializeToArray działa na nieciągłej pamięci (nie na std::array)
    std::string serialized_data;
    if (!msg.SerializeToString(&serialized_data)) {
        return false; // Błąd serializacji
    }

    // 2. Kopiowanie do ramki CAN
    if (serialized_data.length() > frame.data.size()) {
        // Wiadomość Protobuf jest za duża na jedną ramkę CAN
        return false; 
    }

    frame.dlc = serialized_data.length();
    std::copy(serialized_data.begin(), serialized_data.end(), frame.data.begin());

    // 3. Wysłanie
    return hal->transmit(frame);
}

// Konkretna funkcja wysyłająca (używa szablonu)
bool CanInterface::send_pc_main_data(const putm_can::PcMainData& msg) {
    return send_message(msg, 0x10); // PC_MAIN_CAN_ID
}

} // namespace PUTM_CAN