/**
 * @file dbc_registry.cpp
 * @brief Automatically generated DBC Registry.
 * @note DO NOT EDIT. Run database/generate_registry.py to update.
 */

#include "message_handler.hpp"
#include "PUTM_CAN_M.h"
#include "PUTM_CAN_PT.h"

namespace putm_ev_can {

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

#define DBC_ENTRY(PREFIX, MSG_UP, msg_lc) \
    { PREFIX##_##MSG_UP##_FRAME_ID, \
      PREFIX##_##MSG_UP##_LENGTH, \
      (void*)&DbcThunk<PREFIX##_##msg_lc##_t, \
                PREFIX##_##msg_lc##_pack, \
                PREFIX##_##msg_lc##_unpack>::pack_generic, \
      (void*)&DbcThunk<PREFIX##_##msg_lc##_t, \
                PREFIX##_##msg_lc##_pack, \
                PREFIX##_##msg_lc##_unpack>::unpack_generic }

static const DbcEntry DBC_REGISTRY[] = {
    DBC_ENTRY(PUTM_CAN_M, BMS_HV_MAIN, bms_hv_main),
    DBC_ENTRY(PUTM_CAN_M, BMS_LV_MAIN, bms_lv_main),
    DBC_ENTRY(PUTM_CAN_M, BMS_LV_TEMPERATURE, bms_lv_temperature),
    DBC_ENTRY(PUTM_CAN_M, DASHBOARD, dashboard),
    DBC_ENTRY(PUTM_CAN_M, DRIVER_INPUT, driver_input),
    DBC_ENTRY(PUTM_CAN_M, FRONT_DATA, front_data),
    DBC_ENTRY(PUTM_CAN_M, PC_LAP_TIMER_DATA, pc_lap_timer_data),
    DBC_ENTRY(PUTM_CAN_M, PC_MAIN_DATA, pc_main_data),
    DBC_ENTRY(PUTM_CAN_M, PC_TEMPERATURE_DATA, pc_temperature_data),
    DBC_ENTRY(PUTM_CAN_M, PDU_CHANNNEL, pdu_channnel),
    DBC_ENTRY(PUTM_CAN_M, PDU_DATA, pdu_data),
    DBC_ENTRY(PUTM_CAN_M, REARBOX_MISCELLANEOUS, rearbox_miscellaneous),
    DBC_ENTRY(PUTM_CAN_M, REARBOX_SAFETY, rearbox_safety),
    DBC_ENTRY(PUTM_CAN_M, REARBOX_TEMPERATURE, rearbox_temperature),
    DBC_ENTRY(PUTM_CAN_PT, AMK_FRONT_LEFT_ACTUAL_VALUES1, amk_front_left_actual_values1),
    DBC_ENTRY(PUTM_CAN_PT, AMK_FRONT_LEFT_ACTUAL_VALUES2, amk_front_left_actual_values2),
    DBC_ENTRY(PUTM_CAN_PT, AMK_FRONT_LEFT_SETPOINTS1, amk_front_left_setpoints1),
    DBC_ENTRY(PUTM_CAN_PT, AMK_FRONT_RIGHT_ACTUAL_VALUES1, amk_front_right_actual_values1),
    DBC_ENTRY(PUTM_CAN_PT, AMK_FRONT_RIGHT_ACTUAL_VALUES2, amk_front_right_actual_values2),
    DBC_ENTRY(PUTM_CAN_PT, AMK_FRONT_RIGHT_SETPOINTS1, amk_front_right_setpoints1),
    DBC_ENTRY(PUTM_CAN_PT, AMK_REAR_LEFT_ACTUAL_VALUES1, amk_rear_left_actual_values1),
    DBC_ENTRY(PUTM_CAN_PT, AMK_REAR_LEFT_ACTUAL_VALUES2, amk_rear_left_actual_values2),
    DBC_ENTRY(PUTM_CAN_PT, AMK_REAR_LEFT_SETPOINTS1, amk_rear_left_setpoints1),
    DBC_ENTRY(PUTM_CAN_PT, AMK_REAR_RIGHT_ACTUAL_VALUES1, amk_rear_right_actual_values1),
    DBC_ENTRY(PUTM_CAN_PT, AMK_REAR_RIGHT_ACTUAL_VALUES2, amk_rear_right_actual_values2),
    DBC_ENTRY(PUTM_CAN_PT, AMK_REAR_RIGHT_SETPOINTS1, amk_rear_right_setpoints1)
};

const DbcEntry* find_dbc_entry(PUTM_CAN::CanId id) {
    for (const auto& e : DBC_REGISTRY) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

} // namespace putm_ev_can
