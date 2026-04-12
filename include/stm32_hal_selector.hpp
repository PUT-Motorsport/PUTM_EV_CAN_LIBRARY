/**
 * @file stm32_hal_selector.hpp
 * @brief Automatic STM32 family detection and HAL header selection.
 * @details Handles differences between FDCAN (G4/H7) and bxCAN (F4/L4/F7).
 * @author PUT Motorsport
 */

#ifndef STM32_HAL_SELECTOR_HPP
#define STM32_HAL_SELECTOR_HPP

/* * Detect STM32 Family based on compiler definitions.
 * We check for generic family macros (STM32G4xx) AND specific part macros (e.g. STM32G473xx)
 * to ensure compatibility with different build systems (CMake, CubeIDE, Makefile).
 */

// --- STM32 G4 Family (FDCAN) ---
#if defined(STM32G4) || defined(STM32G4xx) || \
    defined(STM32G431xx) || defined(STM32G441xx) || \
    defined(STM32G471xx) || defined(STM32G473xx) || defined(STM32G474xx) || \
    defined(STM32G483xx) || defined(STM32G484xx)
    
    #include "stm32g4xx_hal.h"
    #define PUTM_USE_FDCAN 1

// --- STM32 G0 Family (FDCAN) ---
#elif defined(STM32G0) || defined(STM32G0xx) || \
      defined(STM32G0B1xx) || defined(STM32G0C1xx)
    
    #include "stm32g0xx_hal.h"
    #define PUTM_USE_FDCAN 1

// --- STM32 H7 Family (FDCAN) ---
#elif defined(STM32H7) || defined(STM32H7xx)
    #include "stm32h7xx_hal.h"
    #define PUTM_USE_FDCAN 1

// --- STM32 L5 Family (FDCAN) ---
#elif defined(STM32L5) || defined(STM32L5xx)
    #include "stm32l5xx_hal.h"
    #define PUTM_USE_FDCAN 1

// --- STM32 L4 Family (bxCAN) ---
#elif defined(STM32L4) || defined(STM32L4xx)
    #include "stm32l4xx_hal.h"
    #define PUTM_USE_BXCAN 1

// --- STM32 F4 Family (bxCAN) ---
#elif defined(STM32F4) || defined(STM32F4xx)
    #include "stm32f4xx_hal.h"
    #define PUTM_USE_BXCAN 1

// --- STM32 F7 Family (bxCAN) ---
#elif defined(STM32F7) || defined(STM32F7xx)
    #include "stm32f7xx_hal.h"
    #define PUTM_USE_BXCAN 1

// --- STM32 F1 Family (bxCAN) ---
#elif defined(STM32F1) || defined(STM32F1xx)
    #include "stm32f1xx_hal.h"
    #define PUTM_USE_BXCAN 1

#else
    #error "PUTM CAN LIB: Unknown STM32 Family! Check your preprocessor defines (e.g. STM32G473xx)."
#endif

namespace putm_ev_can {

/**
 * @brief Type alias for the specific HAL Handle Type.
 */
#if defined(PUTM_USE_FDCAN)
    using CanHandleType = FDCAN_HandleTypeDef;
#elif defined(PUTM_USE_BXCAN)
    using CanHandleType = CAN_HandleTypeDef;
#endif

} // namespace putm_ev_can

#endif // STM32_HAL_SELECTOR_HPP