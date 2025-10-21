#ifndef PUTM_CAN_MESSAGE_HANDLER_HPP
#define PUTM_CAN_MESSAGE_HANDLER_HPP

#include <functional>
#include <span>
#include <unordered_map>
#include "can_interface.hpp"
#include "generated/pdu.pb.h" // Wygenerowany plik Protobuf

namespace putm_can {

    // Typy pomocnicze
    using CanId = uint32_t;
    using CallbackFunction = std::function<void(const google::protobuf::Message&)>;

    /**
     * @brief Klasa zarządzająca wiadomościami CAN, obsługująca serializację/deserializację i callbacki.
     * 
     * Ta klasa jest częścią warstwy abstrakcji komunikacji i współpracuje z CanInterface.
     * Używa Protobuf do serializacji/deserializacji wiadomości.
     */
    class MessageHandler {
    public:
        /**
         * @brief Konstruktor domyślny.
         */
        MessageHandler();

        /**
         * @brief Rejestruje callback dla określonego ID CAN.
         * @param id ID wiadomości CAN.
         * @param callback Funkcja callback do wywołania po odebraniu wiadomości.
         * @tparam MsgType Typ wiadomości Protobuf (np. PduChannel).
         */
        template <typename MsgType>
        void register_callback(CanId id, std::function<void(const MsgType&)> callback);

        /**
         * @brief Serializuje wiadomość do bufora CAN (maks. 8 bajtów).
         * @param msg Wiadomość Protobuf do serializacji.
         * @param buffer Bufor wyjściowy (span<uint8_t>).
         * @return true, jeśli serializacja się powiodła, false w przeciwnym razie.
         */
        template <typename MsgType>
        bool serialize(const MsgType& msg, std::span<uint8_t> buffer) const;

        /**
         * @brief Deserializuje wiadomość z bufora CAN.
         * @param id ID wiadomości CAN.
         * @param buffer Bufor wejściowy (span<const uint8_t>).
         * @param[out] msg Obiekt wiadomości Protobuf do wypełnienia.
         * @return true, jeśli deserializacja się powiodła, false w przeciwnym razie.
         */
        template <typename MsgType>
        bool deserialize(CanId id, std::span<const uint8_t> buffer, MsgType& msg) const;

        /**
         * @brief Wywołuje callback dla odebranej wiadomości.
         * @param id ID wiadomości CAN.
         * @param data Surowe dane z bufora CAN.
         */
        void handle_message(CanId id, std::span<const uint8_t> data);

    private:
        // Mapa przechowująca callbacki dla poszczególnych ID
        std::unordered_map<CanId, CallbackFunction> callbacks_;

        // Stałe: maksymalny rozmiar wiadomości CAN (8 bajtów dla standardu)
        static constexpr size_t CAN_MAX_DLC = 8;
    };

} // namespace putm_can

#endif // PUTM_CAN_MESSAGE_HANDLER_HPP