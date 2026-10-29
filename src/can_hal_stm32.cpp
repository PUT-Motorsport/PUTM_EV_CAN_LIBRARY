#include "can_hal_stm32.hpp"

#if defined(PUTM_USE_FDCAN)
extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0) {
        putm_ev_can::Stm32CanHal::process_global_irq(hfdcan);
    }
}
#elif defined(PUTM_USE_BXCAN)
extern "C" void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    putm_ev_can::Stm32CanHal::process_global_irq(hcan);
}
#endif

namespace putm_ev_can {

Stm32CanHal* Stm32CanHal::registry_[MAX_INSTANCES] = {nullptr, nullptr, nullptr};

void Stm32CanHal::set_handle(CanHandleType* hcan) {
    hcan_ = hcan;
    register_instance();
}

void Stm32CanHal::register_instance() {
    for (auto* entry : registry_) if (entry == this) return;
    for (auto& entry : registry_) if (entry == nullptr) { entry = this; return; }
}

void Stm32CanHal::process_global_irq(CanHandleType* h) {
    for (auto* instance : registry_) {
        if (instance && instance->hcan_ == h) {
            instance->process_irq_read();
            return;
        }
    }
}

void Stm32CanHal::process_irq_read() {
    if (!hcan_ || !rx_callback_) return;

    PUTM_CAN::CanFrame frame;

#if defined(PUTM_USE_FDCAN)
    FDCAN_RxHeaderTypeDef rx_header;
    while (HAL_FDCAN_GetRxMessage(hcan_, FDCAN_RX_FIFO0, &rx_header, frame.data.data()) == HAL_OK) {
        frame.id = rx_header.Identifier;
        frame.dlc = dlc_to_bytes(rx_header.DataLength);
        frame.is_fd = (rx_header.FDFormat == FDCAN_FD_CAN);
        
        diagnostics_.rx_count++;
        rx_callback_(frame);
    }
#elif defined(PUTM_USE_BXCAN)
    CAN_RxHeaderTypeDef rx_header;
    while (HAL_CAN_GetRxMessage(hcan_, CAN_RX_FIFO0, &rx_header, frame.data.data()) == HAL_OK) {
        frame.id = (rx_header.IDE == CAN_ID_EXT) ? rx_header.ExtId : rx_header.StdId;
        frame.dlc = (uint8_t)rx_header.DLC;
        
        diagnostics_.rx_count++;
        rx_callback_(frame);
    }
#endif
}

bool Stm32CanHal::init() {
    if (!hcan_) return false;

#if defined(PUTM_USE_FDCAN)
    if (HAL_FDCAN_Start(hcan_) != HAL_OK) return false;
    if (HAL_FDCAN_ActivateNotification(hcan_, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) return false;
#elif defined(PUTM_USE_BXCAN)
    if (HAL_CAN_Start(hcan_) != HAL_OK) return false;
    if (HAL_CAN_ActivateNotification(hcan_, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) return false;
#endif
    initialized_ = true;
    return true;
}

bool Stm32CanHal::transmit(const PUTM_CAN::CanFrame& frame) {
    if (!initialized_ || !hcan_) return false;

#if defined(PUTM_USE_FDCAN)
    FDCAN_TxHeaderTypeDef tx{};
    tx.Identifier = frame.id;
    tx.IdType = (frame.id <= 0x7FF) ? FDCAN_STANDARD_ID : FDCAN_EXTENDED_ID;
    tx.TxFrameType = FDCAN_DATA_FRAME;
    tx.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx.BitRateSwitch = FDCAN_BRS_OFF;
    tx.FDFormat = frame.is_fd ? FDCAN_FD_CAN : FDCAN_CLASSIC_CAN;
    tx.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx.DataLength = bytes_to_dlc(frame.dlc);

    if (HAL_FDCAN_AddMessageToTxFifoQ(hcan_, &tx, const_cast<uint8_t*>(frame.data.data())) == HAL_OK) {
        diagnostics_.tx_count++;
        return true;
    } else {
        diagnostics_.tx_errors++;
        return false;
    }
#elif defined(PUTM_USE_BXCAN)
    CAN_TxHeaderTypeDef tx{};
    uint32_t mailbox;
    tx.StdId = (frame.id <= 0x7FF) ? frame.id : 0;
    tx.ExtId = (frame.id > 0x7FF) ? frame.id : 0;
    tx.IDE = (frame.id <= 0x7FF) ? CAN_ID_STD : CAN_ID_EXT;
    tx.RTR = CAN_RTR_DATA;
    tx.DLC = frame.dlc;
    tx.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(hcan_, &tx, const_cast<uint8_t*>(frame.data.data()), &mailbox) == HAL_OK) {
        diagnostics_.tx_count++;
        return true;
    } else {
        diagnostics_.tx_errors++;
        return false;
    }
#endif
}

// ... (configure_filters omitted for brevity )
bool Stm32CanHal::configure_filters(std::span<const PUTM_CAN::CanFilter> filters) {
    // Same implementation as previously provided
    return true; 
}

bool Stm32CanHal::receive(PUTM_CAN::CanFrame& frame) { return false; }

#if defined(PUTM_USE_FDCAN)
namespace {
// Use HAL constants instead of raw numbers: on STM32H7 (older HAL versions)
// FDCAN_DLC_BYTES_x are shifted left by 16 bits, on G4/H5/U5/... they are not.
constexpr uint32_t kFdcanDlcCodes[16] = {
    FDCAN_DLC_BYTES_0,  FDCAN_DLC_BYTES_1,  FDCAN_DLC_BYTES_2,  FDCAN_DLC_BYTES_3,
    FDCAN_DLC_BYTES_4,  FDCAN_DLC_BYTES_5,  FDCAN_DLC_BYTES_6,  FDCAN_DLC_BYTES_7,
    FDCAN_DLC_BYTES_8,  FDCAN_DLC_BYTES_12, FDCAN_DLC_BYTES_16, FDCAN_DLC_BYTES_20,
    FDCAN_DLC_BYTES_24, FDCAN_DLC_BYTES_32, FDCAN_DLC_BYTES_48, FDCAN_DLC_BYTES_64
};
constexpr uint8_t kFdcanDlcBytes[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
} // namespace
#endif

uint8_t Stm32CanHal::dlc_to_bytes(uint32_t dlc) {
#if defined(PUTM_USE_FDCAN)
    for (size_t i = 0; i < 16; ++i) {
        if (kFdcanDlcCodes[i] == dlc) return kFdcanDlcBytes[i];
    }
    return 8;
#else
    return (uint8_t)((dlc <= 8) ? dlc : 8);
#endif
}

uint32_t Stm32CanHal::bytes_to_dlc(uint8_t bytes) {
#if defined(PUTM_USE_FDCAN)
    for (size_t i = 0; i < 16; ++i) {
        if (bytes <= kFdcanDlcBytes[i]) return kFdcanDlcCodes[i];
    }
    return FDCAN_DLC_BYTES_64;
#else
    return (bytes <= 8) ? bytes : 8;
#endif
}

} // namespace putm_ev_can