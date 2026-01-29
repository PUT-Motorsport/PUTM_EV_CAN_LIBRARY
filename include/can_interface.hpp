/**
 * @file can_interface.hpp
 * @brief High-level CAN logic: DBC mapping and data packing management.
 */

#ifndef PUTM_EV_CAN_INTERFACE_HPP
#define PUTM_EV_CAN_INTERFACE_HPP

#include <cstdint>
#include <span>
#include <cstring>
#include "can_hal.hpp"
#include "PUTM_CAN_M.h"

namespace putm_ev_can {

// Import CanId type for convenience
using PUTM_CAN::CanId;

// ==================================================================================
// DBC REGISTRY SECTION (Must be defined BEFORE CanInterface class)
// ==================================================================================

using PackFn   = int(*)(uint8_t* dst, const void* src, size_t size);
using UnpackFn = int(*)(void* dst, const uint8_t* src, size_t size);

struct DbcEntry {
    CanId    id;
    uint8_t  len;
    PackFn   pack;
    UnpackFn unpack;
};

/**
 * @brief Helper template to cast generated DBC functions to generic pointers.
 */
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

/**
 * @brief Macro to simplify registry entries. Requires PUTM_CAN_M.h macros.
 */
#define DBC_ENTRY(MSG_UP, msg_lc) \
    { PUTM_CAN_M_##MSG_UP##_FRAME_ID, \
      PUTM_CAN_M_##MSG_UP##_LENGTH, \
      &DbcThunk<PUTM_CAN_M_##msg_lc##_t, \
                PUTM_CAN_M_##msg_lc##_pack, \
                PUTM_CAN_M_##msg_lc##_unpack>::pack_generic, \
      &DbcThunk<PUTM_CAN_M_##msg_lc##_t, \
                PUTM_CAN_M_##msg_lc##_pack, \
                PUTM_CAN_M_##msg_lc##_unpack>::unpack_generic }

/**
 * @brief Registry of all supported CAN frames.
 * @note Add new frames here from the DBC file.
 */
static constexpr DbcEntry DBC_REGISTRY[] = {
    DBC_ENTRY(REARBOX_SAFETY,          rearbox_safety),
    DBC_ENTRY(REARBOX_TEMPERATURE,     rearbox_temperature),
    DBC_ENTRY(REARBOX_MISCELLANEOUS,   rearbox_miscellaneous),
    DBC_ENTRY(PC_TEMPERATURE_DATA,     pc_temperature_data),
    DBC_ENTRY(PC_MAIN_DATA,            pc_main_data),
    DBC_ENTRY(PC_LAP_TIMER_DATA,       pc_lap_timer_data),
    DBC_ENTRY(PDU_CHANNNEL,            pdu_channnel),
    DBC_ENTRY(PDU_DATA,                pdu_data),
    DBC_ENTRY(DRIVER_INPUT,            driver_input),
    DBC_ENTRY(FRONT_DATA,              front_data),
    DBC_ENTRY(DASHBOARD,               dashboard),
    DBC_ENTRY(BMS_LV_MAIN,             bms_lv_main),
    DBC_ENTRY(BMS_LV_TEMPERATURE,      bms_lv_temperature),
    DBC_ENTRY(BMS_HV_MAIN,             bms_hv_main)
};

/**
 * @brief Finds a DBC entry by Frame ID.
 * @param id CAN ID to search for.
 * @return Pointer to DbcEntry or nullptr if not found.
 */
inline const DbcEntry* find_dbc_entry(CanId id) {
    for (const auto& e : DBC_REGISTRY)
        if (e.id == id) return &e;
    return nullptr;
}

// ==================================================================================
// HIGH LEVEL INTERFACE
// ==================================================================================

class MessageHandler; // Forward declaration

/**
 * @brief Logical CAN interface.
 * @details Handles packing C++ structures and delegating reception.
 */
class CanInterface {
public:
    virtual ~CanInterface() = default;
    virtual bool init() = 0;
    
    /**
     * @brief Main processing function. Must be called cyclically.
     */
    virtual void process_received_messages() = 0;
    
    virtual bool configure_filters(std::span<const PUTM_CAN::CanFilter> filters) = 0;

    /**
     * @brief Sends a typed message structure.
     * @tparam MsgType Structure type (e.g. PUTM_CAN_1_pc_main_data_t).
     * @param id Frame ID.
     * @param msg Data structure to be packed and sent.
     * @return true on success, false on error or unknown ID.
     */
    template<typename MsgType>
    bool send(CanId id, const MsgType& msg) {
        // find_dbc_entry is now visible here
        const auto* e = find_dbc_entry(id);
        if (!e) return false; 

        uint8_t buffer[8] = {0};
        if (e->pack(buffer, &msg, e->len) < 0) return false; 

        return send_raw(id, std::span<const uint8_t>(buffer, e->len));
    }

protected:
    virtual bool send_raw(CanId id, std::span<const uint8_t> data) = 0;
};

/**
 * @brief Default implementation connecting HAL and MessageHandler.
 */
class DefaultCanInterface : public CanInterface {
public:
    DefaultCanInterface(PUTM_CAN::ICanHal& hal, MessageHandler& mh)
        : hal_(hal), handler_(mh) {}

    // ZMIANA TUTAJ: Tylko deklaracja (Implementation is in .cpp)
    bool init() override;

    void process_received_messages() override; 

    bool configure_filters(std::span<const PUTM_CAN::CanFilter> filters) override {
        return hal_.configure_filters(filters);
    }

protected:
    bool send_raw(CanId id, std::span<const uint8_t> data) override {
        PUTM_CAN::CanFrame frame{};
        frame.id = id;
        frame.dlc = static_cast<uint8_t>(data.size());
        std::memcpy(frame.data.data(), data.data(), frame.dlc);
        return hal_.transmit(frame);
    }

private:
    PUTM_CAN::ICanHal& hal_;
    MessageHandler& handler_;
};

} // namespace putm_ev_can

#endif // PUTM_EV_CAN_INTERFACE_HPP