cmake_minimum_required(VERSION 3.16)
project(PUTM_EV_CAN_LIBRARY LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# --- Backend Detection ---
if(NOT DEFINED PUTM_CAN_BACKEND)
    if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
        set(PUTM_CAN_BACKEND STM32)
    elseif(DEFINED ENV{AMENT_PREFIX_PATH})
        set(PUTM_CAN_BACKEND ROS2)
    else()
        set(PUTM_CAN_BACKEND STM32)
    endif()
endif()

# --- Code Generation ---
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(CAN_GEN_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/database/setup.py")
set(CAN_GEN_WORKDIR "${CMAKE_CURRENT_SOURCE_DIR}/database")

execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CAN_GEN_SCRIPT}"
    WORKING_DIRECTORY "${CAN_GEN_WORKDIR}"
    RESULT_VARIABLE GEN_RESULT
)

if(NOT GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "CAN code generation failed.")
endif()

add_custom_target(
    GenerateCanCode ALL
    COMMAND ${Python3_EXECUTABLE} "${CAN_GEN_SCRIPT}"
    WORKING_DIRECTORY "${CAN_GEN_WORKDIR}"
)

# --- Source Selection ---
file(GLOB GENERATED_DBC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/database/generated/*.c")

set(COMMON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/can_driver.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/message_handler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dbc_registry.cpp"
    ${GENERATED_DBC_SOURCES}
)

set(HAL_SOURCES "")
if(PUTM_CAN_BACKEND STREQUAL "STM32")
    list(APPEND HAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/can_hal_stm32.cpp")
elseif(PUTM_CAN_BACKEND STREQUAL "ROS2")
    list(APPEND HAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/can_hal_ros2.cpp")
endif()

# --- Library Definition ---
add_library(putm_ev_can STATIC ${COMMON_SOURCES} ${HAL_SOURCES})
add_dependencies(putm_ev_can GenerateCanCode)

# --- Includes & Dependencies ---
target_include_directories(putm_ev_can PUBLIC 
    include 
    database/generated
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(PUTM_CAN_BACKEND STREQUAL "STM32")
    target_compile_definitions(putm_ev_can PUBLIC PUTM_CAN_BACKEND_STM32)
    target_link_libraries(putm_ev_can PUBLIC stm32cubemx)

    get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
    
    target_include_directories(putm_ev_can PUBLIC
        "${PROJECT_ROOT_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc"
        "${PROJECT_ROOT_DIR}/Drivers/CMSIS/Device/ST/STM32G4xx/Include"
        "${PROJECT_ROOT_DIR}/Drivers/CMSIS/Include"
    )

elseif(PUTM_CAN_BACKEND STREQUAL "ROS2")
    target_compile_definitions(putm_ev_can PUBLIC PUTM_CAN_BACKEND_ROS2)
    
    find_package(Threads REQUIRED)
    find_package(rclcpp REQUIRED)
    
    target_link_libraries(putm_ev_can PRIVATE 
        Threads::Threads
        rclcpp::rclcpp
    )
endif()