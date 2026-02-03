cmake_minimum_required(VERSION 3.16)

project(PUTM_EV_CAN_LIBRARY LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# =========================================================
# --- Backend Configuration ---
# =========================================================
# Select backend platform (default: STM32)
if(NOT DEFINED PUTM_CAN_BACKEND)
    set(PUTM_CAN_BACKEND STM32)
endif()

message(STATUS "PUTM CAN LIB Backend: ${PUTM_CAN_BACKEND}")

if(PUTM_CAN_BACKEND STREQUAL "STM32")
    add_compile_definitions(PUTM_CAN_BACKEND_STM32)
elseif(PUTM_CAN_BACKEND STREQUAL "ROS2")
    add_compile_definitions(PUTM_CAN_BACKEND_ROS2)
endif()

# =========================================================
# --- Python Integration & Code Generation ---
# =========================================================
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(CAN_GEN_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/dbc/setup.py")
set(CAN_GEN_WORKDIR "${CMAKE_CURRENT_SOURCE_DIR}/dbc")

# 1. Run generation during CONFIGURATION to ensure source files exist for CMake
execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CAN_GEN_SCRIPT}"
    WORKING_DIRECTORY "${CAN_GEN_WORKDIR}"
    RESULT_VARIABLE GEN_RESULT
)

if(NOT GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "Error: CAN code generation failed.")
endif()

# 2. Custom target to check/regenerate DBC files on every build
add_custom_target(
    GenerateCanCode ALL
    COMMAND ${Python3_EXECUTABLE} "${CAN_GEN_SCRIPT}"
    WORKING_DIRECTORY "${CAN_GEN_WORKDIR}"
    COMMENT "Checking and regenerating DBC files..."
)

# =========================================================
# --- Source Files Selection ---
# =========================================================

# Common files (Logic, Router, Facade, Generated C)
file(GLOB_RECURSE COMMON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/can_interface.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/message_handler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/app_can_lib.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dbc/generated/*.c"
)

# Platform-specific HAL implementation
set(HAL_SOURCES "")

if(PUTM_CAN_BACKEND STREQUAL "STM32")
    list(APPEND HAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/can_hal_stm32.cpp")
elseif(PUTM_CAN_BACKEND STREQUAL "ROS2")
    list(APPEND HAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/can_hal_ros2.cpp")
endif()

# =========================================================
# --- Library Definition ---
# =========================================================

add_library(putm_ev_can STATIC ${COMMON_SOURCES} ${HAL_SOURCES})

# Ensure generation runs before compilation
add_dependencies(putm_ev_can GenerateCanCode)

# =========================================================
# --- Dependencies & Includes ---
# =========================================================

target_include_directories(putm_ev_can PUBLIC
    include
    dbc/generated
)

if(PUTM_CAN_BACKEND STREQUAL "STM32")
    # --- STM32 Configuration ---
    target_link_libraries(putm_ev_can PUBLIC stm32cubemx)
    
    # Resolve paths relative to project root (assuming CubeIDE structure)
    get_filename_component(MAIN_H_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
    get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
    
    target_include_directories(putm_ev_can PUBLIC
        "${MAIN_H_DIR}"
        "${PROJECT_ROOT_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc"
        "${PROJECT_ROOT_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy"
        "${PROJECT_ROOT_DIR}/Drivers/CMSIS/Device/ST/STM32G4xx/Include"
        "${PROJECT_ROOT_DIR}/Drivers/CMSIS/Include"
    )

elseif(PUTM_CAN_BACKEND STREQUAL "ROS2")
    # --- Linux/ROS2 Configuration ---
    # Linux requires pthread for the RX simulation thread
    find_package(Threads REQUIRED)
    target_link_libraries(putm_ev_can PRIVATE Threads::Threads)
endif()