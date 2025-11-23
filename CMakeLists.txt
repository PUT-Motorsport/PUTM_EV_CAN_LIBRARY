cmake_minimum_required(VERSION 3.16)

project(PUTM_EV_CAN_LIBRARY LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# --- Backend Configuration ---
# Select backend: STM32 (default) or ROS2
if(NOT DEFINED PUTM_CAN_BACKEND)
    set(PUTM_CAN_BACKEND STM32)
endif()

if(PUTM_CAN_BACKEND STREQUAL "STM32")
    add_compile_definitions(PUTM_CAN_BACKEND_STM32)
elseif(PUTM_CAN_BACKEND STREQUAL "ROS2")
    add_compile_definitions(PUTM_CAN_BACKEND_ROS2)
endif()

# --- Source Files ---
# Recursively find all source files in src/ and dbc/generated/
file(GLOB_RECURSE CAN_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dbc/generated/*.c"
)

add_library(putm_ev_can STATIC ${CAN_SOURCES})
target_link_libraries(putm_ev_can PUBLIC stm32cubemx)
# --- Path Resolution ---
# Calculate absolute paths relative to the library location: Core/Inc/PUTM_EV_CAN_LIBRARY

# 1. Path to 'Core/Inc' (one level up) - required for main.h
get_filename_component(MAIN_H_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

# 2. Path to Project Root (three levels up) - required for Drivers
get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

# Sanity check: verify if main.h exists in the calculated path
message(STATUS "Looking for main.h in: ${MAIN_H_DIR}")

if(EXISTS "${MAIN_H_DIR}/main.h")
    message(STATUS "Success: main.h found.")
else()
    message(WARNING "Warning: main.h not found in ${MAIN_H_DIR}. Check folder structure.")
endif()

# --- Include Directories ---
target_include_directories(putm_ev_can PUBLIC
    include
    dbc/generated
    
    # Path containing main.h
    "${MAIN_H_DIR}"
    
    # Paths to STM32 HAL and CMSIS drivers
    "${PROJECT_ROOT_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc"
    "${PROJECT_ROOT_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy"
    "${PROJECT_ROOT_DIR}/Drivers/CMSIS/Device/ST/STM32G4xx/Include"
    "${PROJECT_ROOT_DIR}/Drivers/CMSIS/Include"
)