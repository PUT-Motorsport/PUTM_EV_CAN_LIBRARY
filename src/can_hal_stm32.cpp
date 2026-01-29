/**
 * @file can_hal_stm32.cpp
 * @brief Hybrid implementation of STM32 HAL (FDCAN / bxCAN) with Interrupt Dispatching.
 */

#include "PUTM_EV_CAN_LIBRARY/include/can_hal_stm32.hpp"

// ============================================================================
// GLOBAL C INTERRUPT HANDLERS (Conditional)
// ============================================================================

#if defined(PUTM_USE_FDCAN)
// STM32G4 / H7
extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0) {
        putm_ev_can::Stm32CanHal::process_global_irq(hfdcan);
    }
}
#elif defined(PUTM_USE_BXCAN)
// STM32F4 / L4
extern "C" void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    putm_ev_can::Stm32CanHal::process_global_irq(hcan);
}
#endif

namespace putm_ev_can {

// ============================================================================
// REGISTRY & DISPATCHER
// ============================================================================

Stm32CanHal* Stm32CanHal::registry_[MAX_INSTANCES] = {nullptr, nullptr, nullptr};

void Stm32CanHal::set_handle(CanHandleType* hcan) {
    hcan_ = hcan;
    register_instance();
}

void Stm32CanHal::register_instance() {
    for (auto* entry : registry_) {
        if (entry == this) return;
    }
    for (auto& entry : registry_) {
        if (entry == nullptr) {
            entry = this;
            return;
        }
    }
}

void Stm32CanHal::process_global_irq(CanHandleType* h) {
    for (auto* instance : registry_) {
        if (instance && instance->hcan_ == h) {
            instance->process_irq_read();
            return;
        }
    }
}

// ============================================================================
// RECEPTION LOGIC (Process IRQ)
// ============================================================================

void Stm32CanHal::process_irq_read() {
    if (!hcan_ || !rx_callback_) return;

    PUTM_CAN::CanFrame frame;

#if defined(PUTM_USE_FDCAN)
    FDCAN_RxHeaderTypeDef rx_header;
    // Drain FDCAN FIFO
    while (HAL_FDCAN_GetRxMessage(hcan_, FDCAN_RX_FIFO0, &rx_header, frame.data.data()) == HAL_OK) {
        frame.id = rx_header.Identifier;
        frame.dlc = dlc_to_bytes(rx_header.DataLength);
        rx_callback_(frame);
    }

#elif defined(PUTM_USE_BXCAN)
    CAN_RxHeaderTypeDef rx_header;
    // Drain bxCAN FIFO
    while (HAL_CAN_GetRxMessage(hcan_, CAN_RX_FIFO0, &rx_header, frame.data.data()) == HAL_OK) {
        frame.id = (rx_header.IDE == CAN_ID_EXT) ? rx_header.ExtId : rx_header.StdId;
        frame.dlc = (uint8_t)rx_header.DLC; // Direct mapping in bxCAN
        rx_callback_(frame);
    }
#endif
}

// ============================================================================
// INITIALIZATION
// ============================================================================

bool Stm32CanHal::init() {
    if (!hcan_) return false;

#if defined(PUTM_USE_FDCAN)
    if (HAL_FDCAN_Start(hcan_) != HAL_OK) return false;
    // Enable "New Message" Interrupt for FIFO 0
    if (HAL_FDCAN_ActivateNotification(hcan_, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
        return false;
    }

#elif defined(PUTM_USE_BXCAN)
    if (HAL_CAN_Start(hcan_) != HAL_OK) return false;
    // Enable "Msg Pending" Interrupt for FIFO 0
    if (HAL_CAN_ActivateNotification(hcan_, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        return false;
    }
#endif

    initialized_ = true;
    return true;
}

// ============================================================================
// TRANSMISSION
// ============================================================================

bool Stm32CanHal::transmit(const PUTM_CAN::CanFrame& frame) {
    if (!initialized_ || !hcan_) return false;

#if defined(PUTM_USE_FDCAN)
    FDCAN_TxHeaderTypeDef tx{};
    tx.Identifier = frame.id;
    tx.IdType = (frame.id <= 0x7FF) ? FDCAN_STANDARD_ID : FDCAN_EXTENDED_ID;
    tx.TxFrameType = FDCAN_DATA_FRAME;
    tx.ErrorStateIndicator = FDCAN_ESI_ACTIVE; 
    tx.BitRateSwitch = FDCAN_BRS_OFF;          
    tx.FDFormat = FDCAN_CLASSIC_CAN;           
    tx.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx.MessageMarker = 0;
    tx.DataLength = bytes_to_dlc(frame.dlc); // Map bytes to FDCAN code

    return HAL_FDCAN_AddMessageToTxFifoQ(hcan_, &tx, const_cast<uint8_t*>(frame.data.data())) == HAL_OK;

#elif defined(PUTM_USE_BXCAN)
    CAN_TxHeaderTypeDef tx{};
    uint32_t mailbox;
    tx.StdId = (frame.id <= 0x7FF) ? frame.id : 0;
    tx.ExtId = (frame.id > 0x7FF) ? frame.id : 0;
    tx.IDE = (frame.id <= 0x7FF) ? CAN_ID_STD : CAN_ID_EXT;
    tx.RTR = CAN_RTR_DATA;
    tx.DLC = frame.dlc;
    tx.TransmitGlobalTime = DISABLE;

    return HAL_CAN_AddTxMessage(hcan_, &tx, const_cast<uint8_t*>(frame.data.data()), &mailbox) == HAL_OK;
#endif
}

// ============================================================================
// BUS STATUS
// ============================================================================

PUTM_CAN::BusStatus Stm32CanHal::get_bus_status() const {
    if (!hcan_) return PUTM_CAN::BusStatus::BUS_OFF;

#if defined(PUTM_USE_FDCAN)
    FDCAN_ProtocolStatusTypeDef status;
    if (HAL_FDCAN_GetProtocolStatus(hcan_, &status) != HAL_OK) return PUTM_CAN::BusStatus::BUS_OFF;
    if (status.BusOff) return PUTM_CAN::BusStatus::BUS_OFF;
    if (status.ErrorPassive || status.Warning) return PUTM_CAN::BusStatus::WARNING;

#elif defined(PUTM_USE_BXCAN)
    uint32_t error = HAL_CAN_GetError(hcan_);
    if (error & (HAL_CAN_ERROR_BOF)) return PUTM_CAN::BusStatus::BUS_OFF;
    if (error & (HAL_CAN_ERROR_EPV | HAL_CAN_ERROR_EWG)) return PUTM_CAN::BusStatus::WARNING;
#endif

    return PUTM_CAN::BusStatus::OK;
}

// ============================================================================
// FILTER CONFIGURATION
// ============================================================================

bool Stm32CanHal::configure_filters(std::span<const PUTM_CAN::CanFilter> filters) {
    if (!hcan_) return false;
    uint32_t idx = 0;

    for (const auto& f : filters) {
#if defined(PUTM_USE_FDCAN)
        FDCAN_FilterTypeDef fd{};
        fd.IdType = f.extended ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
        fd.FilterIndex = idx++;
        fd.FilterType = FDCAN_FILTER_MASK;
        fd.FilterConfig = (f.fifo == 1) ? FDCAN_FILTER_TO_RXFIFO1 : FDCAN_FILTER_TO_RXFIFO0;
        fd.FilterID1 = f.id;
        fd.FilterID2 = f.mask;
        if (HAL_FDCAN_ConfigFilter(hcan_, &fd) != HAL_OK) return false;

#elif defined(PUTM_USE_BXCAN)
        CAN_FilterTypeDef fd{};
        fd.FilterBank = idx++;
        fd.FilterMode = CAN_FILTERMODE_IDMASK;
        fd.FilterScale = CAN_FILTERSCALE_32BIT;
        fd.FilterIdHigh = (f.extended) ? (f.id >> 13) : (f.id << 5);
        fd.FilterIdLow = (f.extended) ? ((f.id << 3) & 0xFFFF) : 0;
        fd.FilterMaskIdHigh = (f.extended) ? (f.mask >> 13) : (f.mask << 5);
        fd.FilterMaskIdLow = (f.extended) ? ((f.mask << 3) & 0xFFFF) : 0;
        fd.FilterFIFOAssignment = (f.fifo == 1) ? CAN_RX_FIFO1 : CAN_RX_FIFO0;
        fd.FilterActivation = ENABLE;
        fd.SlaveStartFilterBank = 14; 

        if (HAL_CAN_ConfigFilter(hcan_, &fd) != HAL_OK) return false;
#endif
    }
    return true;
}

// ============================================================================
// HELPERS / UNUSED MANUAL RECEIVE
// ============================================================================

bool Stm32CanHal::receive(PUTM_CAN::CanFrame& frame) {
    // Unused in Interrupt Mode
    return false;
}

uint8_t Stm32CanHal::dlc_to_bytes(uint32_t dlc) {
#if defined(PUTM_USE_FDCAN)
    switch (dlc) {
        case FDCAN_DLC_BYTES_0: return 0;
        case FDCAN_DLC_BYTES_1: return 1;
        case FDCAN_DLC_BYTES_2: return 2;
        case FDCAN_DLC_BYTES_3: return 3;
        case FDCAN_DLC_BYTES_4: return 4;
        case FDCAN_DLC_BYTES_5: return 5;
        case FDCAN_DLC_BYTES_6: return 6;
        case FDCAN_DLC_BYTES_7: return 7;
        case FDCAN_DLC_BYTES_8: return 8;
        default: return 0;
    }
#else
    return (uint8_t)dlc;
#endif
}

uint32_t Stm32CanHal::bytes_to_dlc(uint8_t bytes) {
#if defined(PUTM_USE_FDCAN)
    switch (bytes) {
        case 0: return FDCAN_DLC_BYTES_0;
        case 1: return FDCAN_DLC_BYTES_1;
        case 2: return FDCAN_DLC_BYTES_2;
        case 3: return FDCAN_DLC_BYTES_3;
        case 4: return FDCAN_DLC_BYTES_4;
        case 5: return FDCAN_DLC_BYTES_5;
        case 6: return FDCAN_DLC_BYTES_6;
        case 7: return FDCAN_DLC_BYTES_7;
        case 8: return FDCAN_DLC_BYTES_8;
        default: return FDCAN_DLC_BYTES_8;
    }
#else
    return bytes;
#endif
}

} // namespace putm_ev_can