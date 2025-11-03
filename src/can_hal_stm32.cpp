/**
 * @file can_hal_stm32.cpp
 * @brief STM32 FDCAN Hardware Abstraction Layer implementation
 * @details Provides STM32-specific CAN implementation using FDCAN peripheral
 * 
 * @author PUTM Team
 * @version 2.0
 * @date 2024
 */

#include "can_interface.hpp"
#include "main.h" // STM32 HAL definitions
#include "fdcan.h"

namespace putm_ev_can {

/**
 * @class Stm32CanHal
 * @brief STM32 FDCAN implementation of ICanHal interface
 */
class Stm32CanHal : public PUTM_CAN::ICanHal {
private:
    FDCAN_HandleTypeDef* hfdcan_;
    bool initialized_;

public:
    /**
     * @brief Constructor
     * @param hfdcan Pointer to FDCAN handle
     */
    Stm32CanHal(FDCAN_HandleTypeDef* hfdcan) 
        : hfdcan_(hfdcan), initialized_(false) {
    }

    /**
     * @brief Initialize FDCAN peripheral
     * @return true if initialization successful
     */
    bool init() override {
        if (hfdcan_ == nullptr) {
            return false;
        }

        // Start FDCAN module
        if (HAL_FDCAN_Start(hfdcan_) != HAL_OK) {
            return false;
        }

        // Activate RX FIFO
        if (HAL_FDCAN_ActivateNotification(hfdcan_, 
                                          FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 
                                          0) != HAL_OK) {
            return false;
        }

        initialized_ = true;
        return true;
    }

    /**
     * @brief Transmit CAN frame via FDCAN
     * @param frame CAN frame to transmit
     * @return true if transmission successful
     */
    bool transmit(const PUTM_CAN::CanFrame& frame) override {
        if (!initialized_ || hfdcan_ == nullptr) {
            return false;
        }

        FDCAN_TxHeaderTypeDef tx_header = {
            .Identifier = frame.id,
            .IdType = (frame.id <= 0x7FF) ? FDCAN_STANDARD_ID : FDCAN_EXTENDED_ID,
            .TxFrameType = FDCAN_DATA_FRAME,
            .DataLength = FDCAN_DLC_BYTES_8, // Always use 8 bytes for simplicity
            .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
            .BitRateSwitch = FDCAN_BRS_OFF,
            .FDFormat = FDCAN_CLASSIC_CAN,
            .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
            .MessageMarker = 0
        };

        // Adjust DLC based on actual data length
        switch(frame.dlc) {
            case 0: tx_header.DataLength = FDCAN_DLC_BYTES_0; break;
            case 1: tx_header.DataLength = FDCAN_DLC_BYTES_1; break;
            case 2: tx_header.DataLength = FDCAN_DLC_BYTES_2; break;
            case 3: tx_header.DataLength = FDCAN_DLC_BYTES_3; break;
            case 4: tx_header.DataLength = FDCAN_DLC_BYTES_4; break;
            case 5: tx_header.DataLength = FDCAN_DLC_BYTES_5; break;
            case 6: tx_header.DataLength = FDCAN_DLC_BYTES_6; break;
            case 7: tx_header.DataLength = FDCAN_DLC_BYTES_7; break;
            case 8: tx_header.DataLength = FDCAN_DLC_BYTES_8; break;
            default: return false;
        }

        // Send message
        return HAL_FDCAN_AddMessageToTxFifoQ(hfdcan_, &tx_header, 
                                           const_cast<uint8_t*>(frame.data.data())) == HAL_OK;
    }

    /**
     * @brief Receive CAN frame from FDCAN (non-blocking)
     * @param[out] frame Received CAN frame
     * @return true if frame received
     */
    bool receive(PUTM_CAN::CanFrame& frame) override {
        if (!initialized_ || hfdcan_ == nullptr) {
            return false;
        }

        FDCAN_RxHeaderTypeDef rx_header;
        
        // Check FIFO 0 for new messages
        if (HAL_FDCAN_GetRxMessage(hfdcan_, FDCAN_RX_FIFO0, &rx_header, 
                                  frame.data.data()) == HAL_OK) {
            frame.id = rx_header.Identifier;
            frame.dlc = convert_dlc_to_bytes(rx_header.DataLength);
            return true;
        }

        return false;
    }

    /**
     * @brief Check if FDCAN is initialized and ready
     * @return true if ready
     */
    bool is_initialized() const override {
        return initialized_ && (hfdcan_ != nullptr);
    }

private:
    /**
     * @brief Convert FDCAN DLC to byte count
     * @param fdcan_dlc FDCAN data length code
     * @return Number of data bytes
     */
    uint8_t convert_dlc_to_bytes(uint32_t fdcan_dlc) {
        switch(fdcan_dlc) {
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
};

} // namespace putm_ev_can