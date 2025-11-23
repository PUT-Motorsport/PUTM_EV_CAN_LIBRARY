/**
 * @file can_hal_stm32.cpp
 * @brief STM32 FDCAN HAL implementation with hardware filters.
 */

#include "can_hal.hpp"
#include "main.h"
#include "stm32g4xx_hal.h"

namespace putm_ev_can {

/**
 * @brief STM32 FDCAN implementation of ICanHal.
 */
class Stm32CanHal : public PUTM_CAN::ICanHal {
public:
    explicit Stm32CanHal(FDCAN_HandleTypeDef* hfdcan)
        : hfdcan_(hfdcan) {}

    bool init() override {
        if (!hfdcan_) return false;
        if (HAL_FDCAN_Start(hfdcan_) != HAL_OK) return false;

        // Enable RX FIFO0 new message interrupt (optional if polling).
        if (HAL_FDCAN_ActivateNotification(
                hfdcan_, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
            return false;
        }
        initialized_ = true;
        return true;
    }

    bool transmit(const PUTM_CAN::CanFrame& frame) override {
        if (!initialized_ || !hfdcan_) return false;

        FDCAN_TxHeaderTypeDef tx{};
        tx.Identifier  = frame.id;
        tx.IdType      = (frame.id <= 0x7FF) ? FDCAN_STANDARD_ID : FDCAN_EXTENDED_ID;
        tx.TxFrameType = FDCAN_DATA_FRAME;
        tx.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
        tx.BitRateSwitch = FDCAN_BRS_OFF;
        tx.FDFormat      = FDCAN_CLASSIC_CAN;
        tx.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
        tx.MessageMarker = 0;

        // DLC mapping
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
            default: return false;
        }

        return HAL_FDCAN_AddMessageToTxFifoQ(
                   hfdcan_, &tx, const_cast<uint8_t*>(frame.data.data())) == HAL_OK;
    }

    bool receive(PUTM_CAN::CanFrame& frame) override {
        if (!initialized_ || !hfdcan_) return false;

        FDCAN_RxHeaderTypeDef rx{};
        if (HAL_FDCAN_GetRxMessage(
                hfdcan_, FDCAN_RX_FIFO0, &rx, frame.data.data()) == HAL_OK) {
            frame.id  = rx.Identifier;
            frame.dlc = dlc_to_bytes(rx.DataLength);
            return true;
        }
        return false;
    }

    bool configure_filters(const std::vector<PUTM_CAN::CanFilter>& filters) override {
        if (!initialized_ || !hfdcan_) return false;

        uint32_t idx = 0;
        for (const auto& f : filters) {
            FDCAN_FilterTypeDef fd{};
            fd.IdType       = f.extended ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
            fd.FilterIndex  = idx++;
            fd.FilterType   = FDCAN_FILTER_MASK;
            fd.FilterConfig = (f.fifo == 1) ? FDCAN_FILTER_TO_RXFIFO1 : FDCAN_FILTER_TO_RXFIFO0;
            fd.FilterID1    = f.id;
            fd.FilterID2    = f.mask;
            if (HAL_FDCAN_ConfigFilter(hfdcan_, &fd) != HAL_OK) return false;
        }
        return true;
    }

    bool is_initialized() const override { return initialized_ && (hfdcan_ != nullptr); }

private:

static uint8_t dlc_to_bytes(uint32_t fdcan_dlc)
    {
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
            // Jeśli kiedyś będziesz obsługiwać CAN FD (powyżej 8 bajtów), dodaj tutaj kolejne case'y.
            // Twoja struktura CanFrame ma jednak bufor std::array<uint8_t, 8>, więc max to 8.
            default: return 0; 
        }
    }

    FDCAN_HandleTypeDef* hfdcan_ = nullptr;
    bool initialized_ = false;
};

} // namespace putm_ev_can
