/**
 * @file can_interface.hpp
 * @brief High-level CAN interface using DBC-generated structures.
 *
 * Provides templated send/receive with automatic pack/unpack from cantools.
 */

#ifndef PUTM_CAN_INTERFACE_HPP
#define PUTM_CAN_INTERFACE_HPP

#include <cstdint>
#include <functional>
#include <span>
#include <array>

namespace putm_can {

/// CAN message identifier type
using CanId = uint32_t;

/**
 * @brief Abstract base class for CAN communication.
 *
 * Uses cantools-generated structures for message packing/unpacking.
 */
class CanInterface {
public:
    /// Virtual destructor
    virtual ~CanInterface() = default;

    /**
     * @brief Sends a CAN message using DBC-generated pack function.
     *
     * @tparam MsgType DBC-generated message structure (e.g., bms_hv_main_t)
     * @param id CAN frame ID
     * @param msg Message to send
     * @return true on success, false on packing/transmit error
     */
    template <typename MsgType>
    bool send(CanId id, const MsgType& msg) {
        std::array<uint8_t, 8> buffer{};
        int len = MsgType##_pack(buffer.data(), &msg, buffer.size());
        if (len < 0) return false;
        return send_raw(id, std::span(buffer.data(), static_cast<size_t>(len)));
    }

    /**
     * @brief Registers a callback for a specific CAN ID.
     *
     * @tparam MsgType DBC-generated message type
     * @param id CAN ID to listen on
     * @param cb Callback function receiving unpacked message
     */
    template <typename MsgType>
    void register_callback(CanId id, std::function<void(const MsgType&)> cb) {
        register_raw_callback(id, [cb](std::span<const uint8_t> data) {
            MsgType msg{};
            if (MsgType##_unpack(&msg, data.data(), data.size()) == 0) {
                cb(msg);
            }
        });
    }

protected:
    /// Send raw CAN frame (implemented by adapter)
    virtual bool send_raw(CanId id, std::span<const uint8_t> data) = 0;

    /// Register raw data callback (implemented by adapter)
    virtual void register_raw_callback(CanId id,
        std::function<void(std::span<const uint8_t>)> cb) = 0;
};

} // namespace putm_can

#endif // PUTM_CAN_INTERFACE_HPP