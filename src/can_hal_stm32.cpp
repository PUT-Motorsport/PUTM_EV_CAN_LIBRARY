/**
 * @file can_hal_stm32.cpp
 * @brief Implementation of STM32G4 FDCAN HAL.
 */

#include "PUTM_EV_CAN_LIBRARY/include/can_hal_stm32.hpp"

namespace putm_ev_can {

// Note: Constructor is default, hfdcan_ is set via set_handle()

bool Stm32CanHal::init() {
    if (!hfdcan_) return false;

    // Start the FDCAN module
    if (HAL_FDCAN_Start(hfdcan_) != HAL_OK) return false;
    
    // Activate Rx FIFO 0 New Message Notification
    // Adjust this if you use interrupts differently
    if (HAL_FDCAN_ActivateNotification(hfdcan_, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
        return false;
    }

    initialized_ = true;
    return true;
}

bool Stm32CanHal::transmit(const PUTM_CAN::CanFrame& frame) {
    if (!initialized_ || !hfdcan_) return false;

    FDCAN_TxHeaderTypeDef tx{};
    tx.Identifier = frame.id;
    tx.IdType = (frame.id <= 0x7FF) ? FDCAN_STANDARD_ID : FDCAN_EXTENDED_ID;
    tx.TxFrameType = FDCAN_DATA_FRAME;
    tx.ErrorStateIndicator = FDCAN_ESI_ACTIVE; // Active error state
    tx.BitRateSwitch = FDCAN_BRS_OFF;          // No bit-rate switching
    tx.FDFormat = FDCAN_CLASSIC_CAN;           // Classic CAN (not FD)
    tx.TxEventFifoControl = FDCAN_NO_TX_EVENTS;// Don't store TX events
    tx.MessageMarker = 0;

    // Map DLC to HAL definition
    switch (frame.dlc) {
        case 0: tx.DataLength = FDCAN_DLC_BYTES_0; break;
        case 1: tx.DataLength = FDCAN_DLC_BYTES_1; break;
        case 2: tx.DataLength = FDCAN_DLC_BYTES_2; break;
        case 3: tx.DataLength = FDCAN_DLC_BYTES_3; break;
        case 4: tx.DataLength = FDCAN_DLC_BYTES_4; break;
        case 5: tx.DataLength = FDCAN_DLC_BYTES_5; break;
        case 6: tx.DataLength = FDCAN_DLC_BYTES_6; break;
        case 7: tx.DataLength = FDCAN_DLC_BYTES_7; break;
        case 8: tx.DataLength = FDCAN_DLC_BYTES_8; break;
        default: return false; // Unsupported DLC
    }

    // Add to TX FIFO
    // Note: const_cast is safe here because HAL copies data to registers
    return HAL_FDCAN_AddMessageToTxFifoQ(hfdcan_, &tx, const_cast<uint8_t*>(frame.data.data())) == HAL_OK;
}

bool Stm32CanHal::receive(PUTM_CAN::CanFrame& frame) {
    if (!initialized_ || !hfdcan_) return false;

    FDCAN_RxHeaderTypeDef rx{};
    
    // Attempt to retrieve message from FIFO 0
    if (HAL_FDCAN_GetRxMessage(hfdcan_, FDCAN_RX_FIFO0, &rx, frame.data.data()) == HAL_OK) {
        frame.id = rx.Identifier;
        frame.dlc = dlc_to_bytes(rx.DataLength);
        return true;
    }
    return false;
}

bool Stm32CanHal::configure_filters(std::span<const PUTM_CAN::CanFilter> filters) {
    if (!hfdcan_) return false;

    uint32_t idx = 0;
    for (const auto& f : filters) {
        FDCAN_FilterTypeDef fd{};
        fd.IdType = f.extended ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
        fd.FilterIndex = idx++;
        fd.FilterType = FDCAN_FILTER_MASK;
        fd.FilterConfig = (f.fifo == 1) ? FDCAN_FILTER_TO_RXFIFO1 : FDCAN_FILTER_TO_RXFIFO0;
        fd.FilterID1 = f.id;
        fd.FilterID2 = f.mask;
        
        if (HAL_FDCAN_ConfigFilter(hfdcan_, &fd) != HAL_OK) return false;
    }
    
    return true;
}

uint8_t Stm32CanHal::dlc_to_bytes(uint32_t fdcan_dlc) {
    switch (fdcan_dlc) {
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
}

} // namespace putm_ev_can