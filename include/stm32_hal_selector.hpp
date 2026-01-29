/**
 * @file stm32_hal_selector.hpp
 * @brief Automatic STM32 family detection and HAL header selection.
 * @details Handles differences between FDCAN (G4/H7) and bxCAN (F4/L4/F7).
 * @author PUT Motorsport
 */

#ifndef STM32_HAL_SELECTOR_HPP
#define STM32_HAL_SELECTOR_HPP

/* * Detect STM32 Family based on compiler definitions.
 * We check for generic family macros (STM32G4xx) AND specific part macros (STM32G484xx)
 * to ensure compatibility with different build systems (CMake, CubeIDE, Makefile).
 */

#if defined(STM32G4) || defined(STM32G4xx) || defined(STM32G431xx) || defined(STM32G474xx) || defined(STM32G484xx)
    #include "stm32g4xx_hal.h"
    #define PUTM_USE_FDCAN 1  ///< FDCAN Peripheral used (G4)

#elif defined(STM32H7) || defined(STM32H7xx)
    #include "stm32h7xx_hal.h"
    #define PUTM_USE_FDCAN 1  ///< FDCAN Peripheral used (H7)

#elif defined(STM32L5) || defined(STM32L5xx)
    #include "stm32l5xx_hal.h"
    #define PUTM_USE_FDCAN 1  ///< FDCAN Peripheral used (L5)

#elif defined(STM32L4) || defined(STM32L4xx)
    #include "stm32l4xx_hal.h"
    #define PUTM_USE_BXCAN 1  ///< Classic bxCAN used (L4)

#elif defined(STM32F4) || defined(STM32F4xx)
    #include "stm32f4xx_hal.h"
    #define PUTM_USE_BXCAN 1  ///< Classic bxCAN used (F4)

#elif defined(STM32F7) || defined(STM32F7xx)
    #include "stm32f7xx_hal.h"
    #define PUTM_USE_BXCAN 1  ///< Classic bxCAN used (F7)

#elif defined(STM32F1) || defined(STM32F1xx)
    #include "stm32f1xx_hal.h"
    #define PUTM_USE_BXCAN 1  ///< Classic bxCAN used (F1)

#else
    // Jeśli kompilator tutaj dotrze, oznacza to brak zdefiniowanego MCU w CMake/IDE.
    // Sprawdź flagi preprocesora (C/C++ Compiler -> Preprocessor -> Defined symbols).
    #error "PUTM CAN LIB: Unknown STM32 Family! Ensure your build system defines the MCU series (e.g. -DSTM32G4xx or -DSTM32G484xx)."
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