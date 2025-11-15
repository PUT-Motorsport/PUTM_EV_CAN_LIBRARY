/**
 * @file can_interface.hpp
 * @brief High-level CAN interface with DBC auto-mapping (no switch-case).
 */

#ifndef PUTM_EV_CAN_INTERFACE_HPP
#define PUTM_EV_CAN_INTERFACE_HPP

#include <cstdint>
#include <span>
#include <algorithm>
#include <type_traits>
#include <functional>
#include "can_hal.hpp"


#include "PUTM_CAN_1.h"

namespace putm_ev_can {

using CanId = uint32_t;

/* ======== DBC registry: auto map for all frames from PUTM_CAN_1.h ======== */

using PackFn   = int(*)(uint8_t* dst, const void* src, size_t size);
using UnpackFn = int(*)(void* dst, const uint8_t* src, size_t size);

struct DbcEntry {
    CanId   id;
    uint8_t len;
    PackFn   pack;
    UnpackFn unpack;
};

template<typename MsgT,
         int(*Pack)(uint8_t*, const MsgT*, size_t),
         int(*Unpack)(MsgT*, const uint8_t*, size_t)>
struct DbcThunk {
    static int pack_generic(uint8_t* d, const void* s, size_t n) {
        return Pack(d, reinterpret_cast<const MsgT*>(s), n);
    }
    static int unpack_generic(void* d, const uint8_t* s, size_t n) {
        return Unpack(reinterpret_cast<MsgT*>(d), s, n);
    }
};

#define DBC_ENTRY(MSG_UP, msg_lc) \
    { PUTM_CAN_1_##MSG_UP##_FRAME_ID, \
      PUTM_CAN_1_##MSG_UP##_LENGTH, \
      &DbcThunk<PUTM_CAN_1_##msg_lc##_t, \
                PUTM_CAN_1_##msg_lc##_pack, \
                PUTM_CAN_1_##msg_lc##_unpack>::pack_generic, \
      &DbcThunk<PUTM_CAN_1_##msg_lc##_t, \
                PUTM_CAN_1_##msg_lc##_pack, \
                PUTM_CAN_1_##msg_lc##_unpack>::unpack_generic }

static constexpr DbcEntry DBC_REGISTRY[] = {
    DBC_ENTRY(REARBOX_SAFETY,          rearbox_safety),
    DBC_ENTRY(REARBOX_TEMPERATURE,     rearbox_temperature),
    DBC_ENTRY(REARBOX_MISCELLANEOUS,   rearbox_miscellaneous),
    DBC_ENTRY(PC_TEMPERATURE_DATA,     pc_temperature_data),
    DBC_ENTRY(PC_MAIN_DATA,            pc_main_data),
    DBC_ENTRY(PC_LAP_TIMER_DATA,       pc_lap_timer_data),
    DBC_ENTRY(PDU_CHANNNEL,            pdu_channnel),     // uwaga: pisownia z DBC
    DBC_ENTRY(PDU_DATA,                pdu_data),
    DBC_ENTRY(DRIVER_INPUT,            driver_input),
    DBC_ENTRY(FRONT_DATA,              front_data),
    DBC_ENTRY(DASHBOARD,               dashboard),
    DBC_ENTRY(BMS_LV_MAIN,             bms_lv_main),
    DBC_ENTRY(BMS_LV_TEMPERATURE,      bms_lv_temperature),
    DBC_ENTRY(BMS_HV_MAIN,             bms_hv_main)
};

inline const DbcEntry* find_dbc_entry(CanId id) {
    for (const auto& e : DBC_REGISTRY)
        if (e.id == id) return &e;
    return nullptr;
}

/* ========================= High-level interface ========================= */

class MessageHandler; // fwd decl (implementacja w message_handler.*)

class CanInterface {
public:
    virtual ~CanInterface() = default;

    /// Initialize the high-level interface (platform may be initialized here).
    virtual bool init() = 0;

    /// Whether interface is ready.
    virtual bool is_ready() const = 0;

    /// Process all received messages (call periodically).
    virtual void process_received_messages() = 0;

    /// Send a DBC-typed message (no switch-case).
    template<typename MsgType>
    bool send(CanId id, const MsgType& msg) {
        const auto* e = find_dbc_entry(id);
        if (!e) return false;

        uint8_t buffer[8] = {0};
        if (e->pack(buffer, &msg, e->len) < 0) return false;

        return send_raw(id, std::span<const uint8_t>(buffer, e->len));
    }

    /// Optional: pass filters to HW (if supported) or ignore (SW fallback elsewhere).
    virtual bool configure_filters(const std::vector<PUTM_CAN::CanFilter>& filters) = 0;

protected:
    /// Low-level raw send to HAL.
    virtual bool send_raw(CanId id, std::span<const uint8_t> data) = 0;
};

} // namespace putm_ev_can

inline CanInterface* make_default_interface(PUTM_CAN::ICanHal* hal, MessageHandler& mh) 
{
     return new DefaultCanInterface(hal, mh);
}

#endif // PUTM_EV_CAN_INTERFACE_HPP
