#ifndef PUTM_CAN_INTERFACE_HPP
#define PUTM_CAN_INTERFACE_HPP

#include <cstdint>
#include <functional>
#include <span>  // C++20 dla buforów
#include "generated/can_ids.pb.h"  // Enum z proto

namespace putm_can {

class CanInterface {
public:
    virtual ~CanInterface() = default;

    // Wysyłanie: prosty interfejs, pakuje protobuf automatycznie
    template <typename MsgType>
    bool send(CanId id, const MsgType& msg) {
        uint8_t buffer[8];
        std::span<uint8_t> buf_span(buffer);
        if (!serialize(msg, buf_span)) return false;
        return send_raw(id, buf_span);
    }

    // Rejestracja callbacku na wiadomość (łatwy odbiór)
    template <typename MsgType>
    void register_callback(CanId id, std::function<void(const MsgType&)> cb) {
        register_raw_callback(id, [cb](std::span<const uint8_t> data) {
            MsgType msg;
            if (deserialize(data, msg)) cb(msg);
        });
    }

protected:
    virtual bool send_raw(CanId id, std::span<const uint8_t> data) = 0;
    virtual void register_raw_callback(CanId id, std::function<void(std::span<const uint8_t>)> cb) = 0;

    // Pomocnicze (w message_handler.cpp)
    template <typename MsgType>
    static bool serialize(const MsgType& msg, std::span<uint8_t> buffer);
    template <typename MsgType>
    static bool deserialize(std::span<const uint8_t> data, MsgType& msg);
};

}  // namespace putm_can
#endif