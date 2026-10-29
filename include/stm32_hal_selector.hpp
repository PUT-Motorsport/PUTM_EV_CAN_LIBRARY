/**
 * @file stm32_hal_selector.hpp
 * @brief Automatic STM32 family detection and HAL header selection.
 * @details Handles differences between FDCAN and bxCAN peripherals.
 *
 *  FDCAN families : G0 (G0B1/G0C1), G4, H5, H7, H7RS, L5, U5, U3, C0 (C092),
 *                   N6, MP1 (Cortex-M4 core), MP2 (Cortex-M33 core)
 *  bxCAN families : F0, F1, F2, F3, F4, F7, L4 / L4+
 *
 * Detection order:
 *   1. Explicit override:  PUTM_FORCE_FDCAN / PUTM_FORCE_BXCAN together with
 *      PUTM_HAL_HEADER, e.g. -DPUTM_FORCE_FDCAN -DPUTM_HAL_HEADER="stm32h7xx_hal.h"
 *   2. Family macros (STM32H7, STM32H7xx, ...), used by some build systems.
 *   3. __has_include() of the family HAL header. CubeMX projects only define
 *      the part number (e.g. STM32H743xx), so this is the path normally taken.
 *
 * @author PUT Motorsport
 */

#ifndef STM32_HAL_SELECTOR_HPP
#define STM32_HAL_SELECTOR_HPP

/* ------------------------------------------------------------------------- */
/* 1. Manual override                                                        */
/* ------------------------------------------------------------------------- */
#if defined(PUTM_HAL_HEADER) && (defined(PUTM_FORCE_FDCAN) || defined(PUTM_FORCE_BXCAN))
    #include PUTM_HAL_HEADER
    #if defined(PUTM_FORCE_FDCAN)
        #define PUTM_USE_FDCAN 1
    #else
        #define PUTM_USE_BXCAN 1
    #endif
    #define PUTM_STM32_FAMILY_NAME "CUSTOM"

/* ------------------------------------------------------------------------- */
/* 2. Family macros                                                          */
/* ------------------------------------------------------------------------- */

// ===== FDCAN =====
// H7RS must be checked before H7 (different HAL: stm32h7rsxx_hal.h)
#elif defined(STM32H7RS) || defined(STM32H7RSxx) || \
      defined(STM32H7R3xx) || defined(STM32H7R7xx) || \
      defined(STM32H7S3xx) || defined(STM32H7S7xx)
    #include "stm32h7rsxx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "H7RS"

#elif defined(STM32H7) || defined(STM32H7xx)
    #include "stm32h7xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "H7"

#elif defined(STM32H5) || defined(STM32H5xx)
    #include "stm32h5xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "H5"

#elif defined(STM32G4) || defined(STM32G4xx) || \
      defined(STM32G431xx) || defined(STM32G441xx) || \
      defined(STM32G471xx) || defined(STM32G473xx) || defined(STM32G474xx) || \
      defined(STM32G483xx) || defined(STM32G484xx) || \
      defined(STM32G491xx) || defined(STM32G4A1xx) || defined(STM32GBK1CB)
    #include "stm32g4xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "G4"

#elif defined(STM32G0) || defined(STM32G0xx) || \
      defined(STM32G0B1xx) || defined(STM32G0C1xx)
    #include "stm32g0xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "G0"

#elif defined(STM32U5) || defined(STM32U5xx)
    #include "stm32u5xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "U5"

#elif defined(STM32U3) || defined(STM32U3xx)
    #include "stm32u3xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "U3"

#elif defined(STM32L5) || defined(STM32L5xx)
    #include "stm32l5xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "L5"

#elif defined(STM32C0) || defined(STM32C0xx) || defined(STM32C092xx) || defined(STM32C091xx)
    #include "stm32c0xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "C0"

#elif defined(STM32N6) || defined(STM32N6xx)
    #include "stm32n6xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "N6"

#elif defined(STM32MP2) || defined(STM32MP2xx)
    #include "stm32mp2xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "MP2"

#elif defined(STM32MP1) || defined(STM32MP1xx)
    #include "stm32mp1xx_hal.h"
    #define PUTM_USE_FDCAN 1
    #define PUTM_STM32_FAMILY_NAME "MP1"

// ===== bxCAN =====
#elif defined(STM32L4) || defined(STM32L4xx)
    #include "stm32l4xx_hal.h"
    #define PUTM_USE_BXCAN 1
    #define PUTM_STM32_FAMILY_NAME "L4"

#elif defined(STM32F7) || defined(STM32F7xx)
    #include "stm32f7xx_hal.h"
    #define PUTM_USE_BXCAN 1
    #define PUTM_STM32_FAMILY_NAME "F7"

#elif defined(STM32F4) || defined(STM32F4xx)
    #include "stm32f4xx_hal.h"
    #define PUTM_USE_BXCAN 1
    #define PUTM_STM32_FAMILY_NAME "F4"

#elif defined(STM32F3) || defined(STM32F3xx)
    #include "stm32f3xx_hal.h"
    #define PUTM_USE_BXCAN 1
    #define PUTM_STM32_FAMILY_NAME "F3"

#elif defined(STM32F2) || defined(STM32F2xx)
    #include "stm32f2xx_hal.h"
    #define PUTM_USE_BXCAN 1
    #define PUTM_STM32_FAMILY_NAME "F2"

#elif defined(STM32F1) || defined(STM32F1xx)
    #include "stm32f1xx_hal.h"
    #define PUTM_USE_BXCAN 1
    #define PUTM_STM32_FAMILY_NAME "F1"

#elif defined(STM32F0) || defined(STM32F0xx)
    #include "stm32f0xx_hal.h"
    #define PUTM_USE_BXCAN 1
    #define PUTM_STM32_FAMILY_NAME "F0"

/* ------------------------------------------------------------------------- */
/* 3. Fallback: detect by HAL header present on the include path             */
/*    (CubeMX defines only the part number, e.g. STM32H743xx)                */
/* ------------------------------------------------------------------------- */
#elif defined(__has_include)

    // ===== FDCAN =====
    #if __has_include("stm32h7rsxx_hal.h")
        #include "stm32h7rsxx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "H7RS"
    #elif __has_include("stm32h7xx_hal.h")
        #include "stm32h7xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "H7"
    #elif __has_include("stm32h5xx_hal.h")
        #include "stm32h5xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "H5"
    #elif __has_include("stm32g4xx_hal.h")
        #include "stm32g4xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "G4"
    #elif __has_include("stm32g0xx_hal.h")
        #include "stm32g0xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "G0"
    #elif __has_include("stm32u5xx_hal.h")
        #include "stm32u5xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "U5"
    #elif __has_include("stm32u3xx_hal.h")
        #include "stm32u3xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "U3"
    #elif __has_include("stm32l5xx_hal.h")
        #include "stm32l5xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "L5"
    #elif __has_include("stm32c0xx_hal.h")
        #include "stm32c0xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "C0"
    #elif __has_include("stm32n6xx_hal.h")
        #include "stm32n6xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "N6"
    #elif __has_include("stm32mp2xx_hal.h")
        #include "stm32mp2xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "MP2"
    #elif __has_include("stm32mp1xx_hal.h")
        #include "stm32mp1xx_hal.h"
        #define PUTM_USE_FDCAN 1
        #define PUTM_STM32_FAMILY_NAME "MP1"

    // ===== bxCAN =====
    #elif __has_include("stm32l4xx_hal.h")
        #include "stm32l4xx_hal.h"
        #define PUTM_USE_BXCAN 1
        #define PUTM_STM32_FAMILY_NAME "L4"
    #elif __has_include("stm32f7xx_hal.h")
        #include "stm32f7xx_hal.h"
        #define PUTM_USE_BXCAN 1
        #define PUTM_STM32_FAMILY_NAME "F7"
    #elif __has_include("stm32f4xx_hal.h")
        #include "stm32f4xx_hal.h"
        #define PUTM_USE_BXCAN 1
        #define PUTM_STM32_FAMILY_NAME "F4"
    #elif __has_include("stm32f3xx_hal.h")
        #include "stm32f3xx_hal.h"
        #define PUTM_USE_BXCAN 1
        #define PUTM_STM32_FAMILY_NAME "F3"
    #elif __has_include("stm32f2xx_hal.h")
        #include "stm32f2xx_hal.h"
        #define PUTM_USE_BXCAN 1
        #define PUTM_STM32_FAMILY_NAME "F2"
    #elif __has_include("stm32f1xx_hal.h")
        #include "stm32f1xx_hal.h"
        #define PUTM_USE_BXCAN 1
        #define PUTM_STM32_FAMILY_NAME "F1"
    #elif __has_include("stm32f0xx_hal.h")
        #include "stm32f0xx_hal.h"
        #define PUTM_USE_BXCAN 1
        #define PUTM_STM32_FAMILY_NAME "F0"
    #else
        #error "PUTM CAN LIB: Unknown STM32 family! No supported stm32xxxx_hal.h found on the include path. Supported: G0 G4 H5 H7 H7RS L5 U5 U3 C0 N6 MP1 MP2 (FDCAN), F0 F1 F2 F3 F4 F7 L4 (bxCAN). Use PUTM_FORCE_FDCAN/PUTM_FORCE_BXCAN + PUTM_HAL_HEADER to override."
    #endif

#else
    #error "PUTM CAN LIB: Unknown STM32 family! Define a family macro (e.g. STM32H7xx) or use PUTM_FORCE_FDCAN/PUTM_FORCE_BXCAN + PUTM_HAL_HEADER."
#endif

/* ------------------------------------------------------------------------- */
/* Sanity checks: the CAN module must be enabled in stm32xxxx_hal_conf.h     */
/* ------------------------------------------------------------------------- */
#if defined(PUTM_USE_FDCAN) && !defined(HAL_FDCAN_MODULE_ENABLED)
    #error "PUTM CAN LIB: FDCAN family detected but HAL_FDCAN_MODULE_ENABLED is not defined. Enable FDCAN in CubeMX."
#endif
#if defined(PUTM_USE_BXCAN) && !defined(HAL_CAN_MODULE_ENABLED)
    #error "PUTM CAN LIB: bxCAN family detected but HAL_CAN_MODULE_ENABLED is not defined. Enable CAN in CubeMX (not CAN_LEGACY)."
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

/** @brief Human readable name of the detected family (e.g. "H7"). */
inline constexpr const char* kStm32FamilyName = PUTM_STM32_FAMILY_NAME;

} // namespace putm_ev_can

#endif // STM32_HAL_SELECTOR_HPP
