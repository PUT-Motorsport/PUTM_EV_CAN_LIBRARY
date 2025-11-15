cmake_minimum_required(VERSION 3.16)

project(PUTM_EV_CAN_LIBRARY LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# ─────────────────────────────────────────
# ✅ WYBÓR BACKENDU (STM32 / ROS2)
# ─────────────────────────────────────────
if(NOT DEFINED PUTM_CAN_BACKEND)
    set(PUTM_CAN_BACKEND STM32)      # domyślnie STM32
endif()

message(STATUS "💡 CAN backend: ${PUTM_CAN_BACKEND}")

if(PUTM_CAN_BACKEND STREQUAL "STM32")
    add_compile_definitions(PUTM_CAN_BACKEND_STM32)
elseif(PUTM_CAN_BACKEND STREQUAL "ROS2")
    add_compile_definitions(PUTM_CAN_BACKEND_ROS2)
else()
    message(FATAL_ERROR "Unknown backend! Use STM32 or ROS2.")
endif()

# Źródła biblioteki
file(GLOB_RECURSE CAN_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dbc/generated/*.c"
)

add_library(putm_ev_can STATIC ${CAN_SOURCES})

target_include_directories(putm_ev_can PUBLIC
    include
    dbc/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32G4xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32G4xx/Include
)
)
