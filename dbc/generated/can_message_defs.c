/**
 * @file can_message_defs.c
 * @brief CAN encode/decode implementations for STM32 FDCAN
 * @warning Implement bit packing/unpacking according to your DBC file
 * 
 * @generated_by DBC Code Generator
 */

#include "can_message_defs.h"

/**
 * @brief Encode Rearbox_Safety for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Rearbox_Safety_encode(uint8_t data_out[8], const PUTM_CAN_RearboxSafety* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x25 - Rearbox_Safety
    // safety_tsmp: start=0, length=1
    // safety_rfu: start=1, length=1
    // safety_hv_battery: start=2, length=1
    // safety_inv_hv: start=3, length=1
    // safety_hvd: start=4, length=1
    // safety_inv: start=5, length=1
    // safety_wheel_fl: start=6, length=1
    // safety_wheel_fr: start=7, length=1
    // safety_wheel_rl: start=8, length=1
    // safety_wheel_rr: start=9, length=1
    // safety_suspension_rl: start=10, length=1
    // safety_suspension_rr: start=11, length=1
    // safety_motor_front: start=12, length=1
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Rearbox_Safety from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Rearbox_Safety_decode(const uint8_t data_in[8], PUTM_CAN_RearboxSafety* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x25 - Rearbox_Safety
    // safety_tsmp: start=0, length=1
    // msg->safety_tsmp = (data_in[0] >> 0) & 0x01;
    // safety_rfu: start=1, length=1
    // msg->safety_rfu = (data_in[0] >> 1) & 0x01;
    // safety_hv_battery: start=2, length=1
    // msg->safety_hv_battery = (data_in[0] >> 2) & 0x01;
    // safety_inv_hv: start=3, length=1
    // msg->safety_inv_hv = (data_in[0] >> 3) & 0x01;
    // safety_hvd: start=4, length=1
    // msg->safety_hvd = (data_in[0] >> 4) & 0x01;
    // safety_inv: start=5, length=1
    // msg->safety_inv = (data_in[0] >> 5) & 0x01;
    // safety_wheel_fl: start=6, length=1
    // msg->safety_wheel_fl = (data_in[0] >> 6) & 0x01;
    // safety_wheel_fr: start=7, length=1
    // msg->safety_wheel_fr = (data_in[0] >> 7) & 0x01;
    // safety_wheel_rl: start=8, length=1
    // msg->safety_wheel_rl = (data_in[1] >> 0) & 0x01;
    // safety_wheel_rr: start=9, length=1
    // msg->safety_wheel_rr = (data_in[1] >> 1) & 0x01;
    // safety_suspension_rl: start=10, length=1
    // msg->safety_suspension_rl = (data_in[1] >> 2) & 0x01;
    // safety_suspension_rr: start=11, length=1
    // msg->safety_suspension_rr = (data_in[1] >> 3) & 0x01;
    // safety_motor_front: start=12, length=1
    // msg->safety_motor_front = (data_in[1] >> 4) & 0x01;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Rearbox_Temperature for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Rearbox_Temperature_encode(uint8_t data_out[8], const PUTM_CAN_RearboxTemperature* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x26 - Rearbox_Temperature
    // mono_temperature: start=0, length=8
    // coolant_temperature_in: start=8, length=8
    // coolant_temperature_out: start=16, length=8
    // oil_temperature_l: start=24, length=8
    // oil_temperature_r: start=32, length=8
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Rearbox_Temperature from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Rearbox_Temperature_decode(const uint8_t data_in[8], PUTM_CAN_RearboxTemperature* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x26 - Rearbox_Temperature
    // mono_temperature: start=0, length=8
    // msg->mono_temperature = (data_in[0] >> 0) & 0xFF;
    // coolant_temperature_in: start=8, length=8
    // msg->coolant_temperature_in = (data_in[1] >> 0) & 0xFF;
    // coolant_temperature_out: start=16, length=8
    // msg->coolant_temperature_out = (data_in[2] >> 0) & 0xFF;
    // oil_temperature_l: start=24, length=8
    // msg->oil_temperature_l = (data_in[3] >> 0) & 0xFF;
    // oil_temperature_r: start=32, length=8
    // msg->oil_temperature_r = (data_in[4] >> 0) & 0xFF;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Rearbox_Miscellaneous for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Rearbox_Miscellaneous_encode(uint8_t data_out[8], const PUTM_CAN_RearboxMiscellaneous* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x27 - Rearbox_Miscellaneous
    // coolant_pressure_in: start=0, length=8
    // coolant_pressure_out: start=8, length=8
    // suspension_l: start=16, length=16
    // suspension_r: start=32, length=16
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Rearbox_Miscellaneous from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Rearbox_Miscellaneous_decode(const uint8_t data_in[8], PUTM_CAN_RearboxMiscellaneous* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x27 - Rearbox_Miscellaneous
    // coolant_pressure_in: start=0, length=8
    // msg->coolant_pressure_in = (data_in[0] >> 0) & 0xFF;
    // coolant_pressure_out: start=8, length=8
    // msg->coolant_pressure_out = (data_in[1] >> 0) & 0xFF;
    // suspension_l: start=16, length=16
    // msg->suspension_l = (data_in[2] >> 0) & 0xFFFF;
    // suspension_r: start=32, length=16
    // msg->suspension_r = (data_in[4] >> 0) & 0xFFFF;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Pc_TemperatureData for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Pc_TemperatureData_encode(uint8_t data_out[8], const PUTM_CAN_PcTemperaturedata* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x60 - Pc_TemperatureData
    // rearRightInverterTemperature: start=0, length=8
    // rearLeftInverterTemperature: start=8, length=8
    // rearRightMotorTemperature: start=16, length=8
    // rearLeftMotorTemperature: start=24, length=8
    // frontRightInverterTemperature: start=32, length=8
    // frontLeftInverterTemperature: start=40, length=8
    // frontRightMotorTemperature: start=48, length=8
    // frontLeftMotorTemperature: start=56, length=8
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Pc_TemperatureData from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Pc_TemperatureData_decode(const uint8_t data_in[8], PUTM_CAN_PcTemperaturedata* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x60 - Pc_TemperatureData
    // rearRightInverterTemperature: start=0, length=8
    // msg->rearRightInverterTemperature = (data_in[0] >> 0) & 0xFF;
    // rearLeftInverterTemperature: start=8, length=8
    // msg->rearLeftInverterTemperature = (data_in[1] >> 0) & 0xFF;
    // rearRightMotorTemperature: start=16, length=8
    // msg->rearRightMotorTemperature = (data_in[2] >> 0) & 0xFF;
    // rearLeftMotorTemperature: start=24, length=8
    // msg->rearLeftMotorTemperature = (data_in[3] >> 0) & 0xFF;
    // frontRightInverterTemperature: start=32, length=8
    // msg->frontRightInverterTemperature = (data_in[4] >> 0) & 0xFF;
    // frontLeftInverterTemperature: start=40, length=8
    // msg->frontLeftInverterTemperature = (data_in[5] >> 0) & 0xFF;
    // frontRightMotorTemperature: start=48, length=8
    // msg->frontRightMotorTemperature = (data_in[6] >> 0) & 0xFF;
    // frontLeftMotorTemperature: start=56, length=8
    // msg->frontLeftMotorTemperature = (data_in[7] >> 0) & 0xFF;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Pc_MainData for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Pc_MainData_encode(uint8_t data_out[8], const PUTM_CAN_PcMaindata* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x10 - Pc_MainData
    // vehicleSpeed: start=0, length=16
    // torque_current: start=16, length=16
    // invertersReady: start=32, length=1
    // rtd: start=33, length=1
    // inv_FL_status: start=34, length=1
    // inv_FR_status: start=35, length=1
    // inv_RL_status: start=36, length=1
    // inv_RR_status: start=37, length=1
    // inv_FL_error: start=38, length=1
    // inv_FR_error: start=39, length=1
    // inv_RL_error: start=40, length=1
    // inv_RR_error: start=41, length=1
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Pc_MainData from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Pc_MainData_decode(const uint8_t data_in[8], PUTM_CAN_PcMaindata* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x10 - Pc_MainData
    // vehicleSpeed: start=0, length=16
    // msg->vehicleSpeed = (data_in[0] >> 0) & 0xFFFF;
    // torque_current: start=16, length=16
    // msg->torque_current = (data_in[2] >> 0) & 0xFFFF;
    // invertersReady: start=32, length=1
    // msg->invertersReady = (data_in[4] >> 0) & 0x01;
    // rtd: start=33, length=1
    // msg->rtd = (data_in[4] >> 1) & 0x01;
    // inv_FL_status: start=34, length=1
    // msg->inv_FL_status = (data_in[4] >> 2) & 0x01;
    // inv_FR_status: start=35, length=1
    // msg->inv_FR_status = (data_in[4] >> 3) & 0x01;
    // inv_RL_status: start=36, length=1
    // msg->inv_RL_status = (data_in[4] >> 4) & 0x01;
    // inv_RR_status: start=37, length=1
    // msg->inv_RR_status = (data_in[4] >> 5) & 0x01;
    // inv_FL_error: start=38, length=1
    // msg->inv_FL_error = (data_in[4] >> 6) & 0x01;
    // inv_FR_error: start=39, length=1
    // msg->inv_FR_error = (data_in[4] >> 7) & 0x01;
    // inv_RL_error: start=40, length=1
    // msg->inv_RL_error = (data_in[5] >> 0) & 0x01;
    // inv_RR_error: start=41, length=1
    // msg->inv_RR_error = (data_in[5] >> 1) & 0x01;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Pc_LapTimerData for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Pc_LapTimerData_encode(uint8_t data_out[8], const PUTM_CAN_PcLaptimerdata* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x11 - Pc_LapTimerData
    // currentLapTime: start=0, length=16
    // bestLapTime: start=16, length=16
    // delta_time: start=32, length=16
    // lap_counter: start=48, length=8
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Pc_LapTimerData from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Pc_LapTimerData_decode(const uint8_t data_in[8], PUTM_CAN_PcLaptimerdata* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x11 - Pc_LapTimerData
    // currentLapTime: start=0, length=16
    // msg->currentLapTime = (data_in[0] >> 0) & 0xFFFF;
    // bestLapTime: start=16, length=16
    // msg->bestLapTime = (data_in[2] >> 0) & 0xFFFF;
    // delta_time: start=32, length=16
    // msg->delta_time = (data_in[4] >> 0) & 0xFFFF;
    // lap_counter: start=48, length=8
    // msg->lap_counter = (data_in[6] >> 0) & 0xFF;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Pdu_Channnel for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Pdu_Channnel_encode(uint8_t data_out[8], const PUTM_CAN_PduChannnel* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x30 - Pdu_Channnel
    // pc_status: start=0, length=2
    // fan_status: start=2, length=2
    // pump_status: start=4, length=2
    // inverter_status: start=6, length=2
    // fbox_status: start=8, length=2
    // sdc_status: start=10, length=2
    // dash_status: start=12, length=2
    // tsal_hv_status: start=14, length=2
    // rbox_diagport_brake_l_status: start=16, length=2
    // brake_ir_air_status: start=18, length=2
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Pdu_Channnel from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Pdu_Channnel_decode(const uint8_t data_in[8], PUTM_CAN_PduChannnel* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x30 - Pdu_Channnel
    // pc_status: start=0, length=2
    // msg->pc_status = (data_in[0] >> 0) & 0x03;
    // fan_status: start=2, length=2
    // msg->fan_status = (data_in[0] >> 2) & 0x03;
    // pump_status: start=4, length=2
    // msg->pump_status = (data_in[0] >> 4) & 0x03;
    // inverter_status: start=6, length=2
    // msg->inverter_status = (data_in[0] >> 6) & 0x03;
    // fbox_status: start=8, length=2
    // msg->fbox_status = (data_in[1] >> 0) & 0x03;
    // sdc_status: start=10, length=2
    // msg->sdc_status = (data_in[1] >> 2) & 0x03;
    // dash_status: start=12, length=2
    // msg->dash_status = (data_in[1] >> 4) & 0x03;
    // tsal_hv_status: start=14, length=2
    // msg->tsal_hv_status = (data_in[1] >> 6) & 0x03;
    // rbox_diagport_brake_l_status: start=16, length=2
    // msg->rbox_diagport_brake_l_status = (data_in[2] >> 0) & 0x03;
    // brake_ir_air_status: start=18, length=2
    // msg->brake_ir_air_status = (data_in[2] >> 2) & 0x03;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Pdu_Data for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Pdu_Data_encode(uint8_t data_out[8], const PUTM_CAN_PduData* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x31 - Pdu_Data
    // pc_current: start=0, length=8
    // pump_current: start=8, length=8
    // fan_current: start=16, length=8
    // inverter_current: start=24, length=8
    // fbox_current: start=32, length=8
    // sdc_current: start=40, length=8
    // total_current: start=48, length=16
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Pdu_Data from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Pdu_Data_decode(const uint8_t data_in[8], PUTM_CAN_PduData* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x31 - Pdu_Data
    // pc_current: start=0, length=8
    // msg->pc_current = (data_in[0] >> 0) & 0xFF;
    // pump_current: start=8, length=8
    // msg->pump_current = (data_in[1] >> 0) & 0xFF;
    // fan_current: start=16, length=8
    // msg->fan_current = (data_in[2] >> 0) & 0xFF;
    // inverter_current: start=24, length=8
    // msg->inverter_current = (data_in[3] >> 0) & 0xFF;
    // fbox_current: start=32, length=8
    // msg->fbox_current = (data_in[4] >> 0) & 0xFF;
    // sdc_current: start=40, length=8
    // msg->sdc_current = (data_in[5] >> 0) & 0xFF;
    // total_current: start=48, length=16
    // msg->total_current = (data_in[6] >> 0) & 0xFFFF;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode DriverInput for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void DriverInput_encode(uint8_t data_out[8], const PUTM_CAN_Driverinput* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x5 - DriverInput
    // pedalPosition: start=0, length=16
    // brakePressureFront: start=16, length=16
    // brakePressureRear: start=32, length=16
    // steeringWheelPosition: start=48, length=16
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode DriverInput from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void DriverInput_decode(const uint8_t data_in[8], PUTM_CAN_Driverinput* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x5 - DriverInput
    // pedalPosition: start=0, length=16
    // msg->pedalPosition = (data_in[0] >> 0) & 0xFFFF;
    // brakePressureFront: start=16, length=16
    // msg->brakePressureFront = (data_in[2] >> 0) & 0xFFFF;
    // brakePressureRear: start=32, length=16
    // msg->brakePressureRear = (data_in[4] >> 0) & 0xFFFF;
    // steeringWheelPosition: start=48, length=16
    // msg->steeringWheelPosition = (data_in[6] >> 0) & 0xFFFF;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Front_Data for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Front_Data_encode(uint8_t data_out[8], const PUTM_CAN_FrontData* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x35 - Front_Data
    // sense_left_kill: start=0, length=1
    // sense_right_kill: start=1, length=1
    // sense_driver_kill: start=2, length=1
    // sense_inertia: start=3, length=1
    // sense_bspd: start=4, length=1
    // sense_overtravel: start=5, length=1
    // safety_suspension_fl: start=6, length=1
    // safety_suspension_fr: start=7, length=1
    // is_braking: start=8, length=1
    // apps: start=9, length=1
    // apps_implausibility: start=10, length=8
    // frontLeftSuspension: start=18, length=16
    // frontRightSuspension: start=34, length=16
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Front_Data from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Front_Data_decode(const uint8_t data_in[8], PUTM_CAN_FrontData* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x35 - Front_Data
    // sense_left_kill: start=0, length=1
    // msg->sense_left_kill = (data_in[0] >> 0) & 0x01;
    // sense_right_kill: start=1, length=1
    // msg->sense_right_kill = (data_in[0] >> 1) & 0x01;
    // sense_driver_kill: start=2, length=1
    // msg->sense_driver_kill = (data_in[0] >> 2) & 0x01;
    // sense_inertia: start=3, length=1
    // msg->sense_inertia = (data_in[0] >> 3) & 0x01;
    // sense_bspd: start=4, length=1
    // msg->sense_bspd = (data_in[0] >> 4) & 0x01;
    // sense_overtravel: start=5, length=1
    // msg->sense_overtravel = (data_in[0] >> 5) & 0x01;
    // safety_suspension_fl: start=6, length=1
    // msg->safety_suspension_fl = (data_in[0] >> 6) & 0x01;
    // safety_suspension_fr: start=7, length=1
    // msg->safety_suspension_fr = (data_in[0] >> 7) & 0x01;
    // is_braking: start=8, length=1
    // msg->is_braking = (data_in[1] >> 0) & 0x01;
    // apps: start=9, length=1
    // msg->apps = (data_in[1] >> 1) & 0x01;
    // apps_implausibility: start=10, length=8
    // msg->apps_implausibility = (data_in[1] >> 2) & 0xFF;
    // frontLeftSuspension: start=18, length=16
    // msg->frontLeftSuspension = (data_in[2] >> 2) & 0xFFFF;
    // frontRightSuspension: start=34, length=16
    // msg->frontRightSuspension = (data_in[4] >> 2) & 0xFFFF;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Dashboard for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Dashboard_encode(uint8_t data_out[8], const PUTM_CAN_Dashboard* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x15 - Dashboard
    // ready_to_drive_button: start=0, length=1
    // ts_activation_button: start=1, length=1
    // user_button: start=2, length=1
    // drs_button: start=3, length=1
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Dashboard from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Dashboard_decode(const uint8_t data_in[8], PUTM_CAN_Dashboard* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x15 - Dashboard
    // ready_to_drive_button: start=0, length=1
    // msg->ready_to_drive_button = (data_in[0] >> 0) & 0x01;
    // ts_activation_button: start=1, length=1
    // msg->ts_activation_button = (data_in[0] >> 1) & 0x01;
    // user_button: start=2, length=1
    // msg->user_button = (data_in[0] >> 2) & 0x01;
    // drs_button: start=3, length=1
    // msg->drs_button = (data_in[0] >> 3) & 0x01;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Bms_LV_main for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Bms_LV_main_encode(uint8_t data_out[8], const PUTM_CAN_BmsLvMain* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x55 - Bms_LV_main
    // voltage_sum: start=0, length=16
    // soc: start=16, length=8
    // temp_avg: start=24, length=8
    // current: start=32, length=8
    // device_state: start=40, length=8
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Bms_LV_main from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Bms_LV_main_decode(const uint8_t data_in[8], PUTM_CAN_BmsLvMain* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x55 - Bms_LV_main
    // voltage_sum: start=0, length=16
    // msg->voltage_sum = (data_in[0] >> 0) & 0xFFFF;
    // soc: start=16, length=8
    // msg->soc = (data_in[2] >> 0) & 0xFF;
    // temp_avg: start=24, length=8
    // msg->temp_avg = (data_in[3] >> 0) & 0xFF;
    // current: start=32, length=8
    // msg->current = (data_in[4] >> 0) & 0xFF;
    // device_state: start=40, length=8
    // msg->device_state = (data_in[5] >> 0) & 0xFF;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Bms_LV_temperature for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Bms_LV_temperature_encode(uint8_t data_out[8], const PUTM_CAN_BmsLvTemperature* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x56 - Bms_LV_temperature
    // temp_1: start=0, length=8
    // temp_2: start=8, length=8
    // temp_3: start=16, length=8
    // temp_4: start=24, length=8
    // temp_5: start=32, length=8
    // temp_6: start=40, length=8
    // temp_7: start=48, length=8
    // temp_8: start=56, length=8
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Bms_LV_temperature from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Bms_LV_temperature_decode(const uint8_t data_in[8], PUTM_CAN_BmsLvTemperature* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x56 - Bms_LV_temperature
    // temp_1: start=0, length=8
    // msg->temp_1 = (data_in[0] >> 0) & 0xFF;
    // temp_2: start=8, length=8
    // msg->temp_2 = (data_in[1] >> 0) & 0xFF;
    // temp_3: start=16, length=8
    // msg->temp_3 = (data_in[2] >> 0) & 0xFF;
    // temp_4: start=24, length=8
    // msg->temp_4 = (data_in[3] >> 0) & 0xFF;
    // temp_5: start=32, length=8
    // msg->temp_5 = (data_in[4] >> 0) & 0xFF;
    // temp_6: start=40, length=8
    // msg->temp_6 = (data_in[5] >> 0) & 0xFF;
    // temp_7: start=48, length=8
    // msg->temp_7 = (data_in[6] >> 0) & 0xFF;
    // temp_8: start=56, length=8
    // msg->temp_8 = (data_in[7] >> 0) & 0xFF;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

/**
 * @brief Encode Bms_HV_main for STM32 FDCAN
 * @param[out] data_out Output buffer
 * @param[in] msg Message to encode
 */
void Bms_HV_main_encode(uint8_t data_out[8], const PUTM_CAN_BmsHvMain* msg)
{
    // Clear buffer
    memset(data_out, 0, 8);

    // STM32 FDCAN optimized encoding
    // Message ID: 0x45 - Bms_HV_main
    // voltage_sum: start=0, length=16
    // current: start=16, length=16
    // temp_max: start=32, length=8
    // temp_avg: start=40, length=8
    // soc: start=48, length=10
    // ok: start=58, length=1
    // precharge: start=59, length=1
    // TODO: Implement bit packing for STM32 FDCAN
}

/**
 * @brief Decode Bms_HV_main from STM32 FDCAN
 * @param[in] data_in Input buffer
 * @param[out] msg Message to populate
 */
void Bms_HV_main_decode(const uint8_t data_in[8], PUTM_CAN_BmsHvMain* msg)
{
    // STM32 FDCAN optimized decoding
    // Message ID: 0x45 - Bms_HV_main
    // voltage_sum: start=0, length=16
    // msg->voltage_sum = (data_in[0] >> 0) & 0xFFFF;
    // current: start=16, length=16
    // msg->current = (data_in[2] >> 0) & 0xFFFF;
    // temp_max: start=32, length=8
    // msg->temp_max = (data_in[4] >> 0) & 0xFF;
    // temp_avg: start=40, length=8
    // msg->temp_avg = (data_in[5] >> 0) & 0xFF;
    // soc: start=48, length=10
    // msg->soc = (data_in[6] >> 0) & 0x3FF;
    // ok: start=58, length=1
    // msg->ok = (data_in[7] >> 2) & 0x01;
    // precharge: start=59, length=1
    // msg->precharge = (data_in[7] >> 3) & 0x01;
    // TODO: Implement bit unpacking for STM32 FDCAN
}

