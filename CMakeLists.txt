cmake_minimum_required(VERSION 3.16)

project(PUTM_EV_CAN_LIBRARY LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# =========================================================
# --- 1. Backend Auto-Detection ---
# =========================================================

# Allow manual override via -DPUTM_CAN_BACKEND=...
if(NOT DEFINED PUTM_CAN_BACKEND)

    # 1. Priority: Check for embedded toolchain (bare-metal system)
    if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
        message(STATUS "Auto-detect: Generic system (STM32).")
        set(PUTM_CAN_BACKEND STM32)

    # 2. Check for ROS 2 environment variables
    elseif(DEFINED ENV{AMENT_PREFIX_PATH})
        message(STATUS "Auto-detect: ROS 2 environment.")
        set(PUTM_CAN_BACKEND ROS2)

    # 3. Default fallback
    else()
        message(WARNING "Auto-detect failed. Defaulting to STM32.")
        set(PUTM_CAN_BACKEND STM32)
    endif()

endif()

# =========================================================
# --- 2. Code Generation (Python) ---
# =========================================================
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(CAN_GEN_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/dbc/setup.py")
set(CAN_GEN_WORKDIR "${CMAKE_CURRENT_SOURCE_DIR}/dbc")

# Execute generation during the configuration phase.
# This ensures that the .c files exist before CMake creates the file list.
execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CAN_GEN_SCRIPT}"
    WORKING_DIRECTORY "${CAN_GEN_WORKDIR}"
    RESULT_VARIABLE GEN_RESULT
)

if(NOT GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "Error: CAN code generation failed.")
endif()

# Create a custom target to re-run generation on every build if needed.
add_custom_target(
    GenerateCanCode ALL
    COMMAND ${Python3_EXECUTABLE} "${CAN_GEN_SCRIPT}"
    WORKING_DIRECTORY "${CAN_GEN_WORKDIR}"
    COMMENT "Checking and regenerating DBC files..."
)

# =========================================================
# --- 3. Source Selection ---
# =========================================================

# Find generated C files from the DBC folder
file(GLOB GENERATED_DBC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dbc/generated/*.c")

# Define core library sources
set(COMMON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/can_driver.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/message_handler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dbc_registry.cpp"
    ${GENERATED_DBC_SOURCES}
)

# Select HAL source based on the backend
set(HAL_SOURCES "")
if(PUTM_CAN_BACKEND STREQUAL "STM32")
    list(APPEND HAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/can_hal_stm32.cpp")
elseif(PUTM_CAN_BACKEND STREQUAL "ROS2")
    list(APPEND HAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/can_hal_ros2.cpp")
endif()

# =========================================================
# --- 4. Library Definition ---
# =========================================================

# Create the static library
add_library(putm_ev_can STATIC ${COMMON_SOURCES} ${HAL_SOURCES})

# Ensure code generation runs before the library is compiled
add_dependencies(putm_ev_can GenerateCanCode)

# =========================================================
# --- 5. Configuration & Dependencies ---
# =========================================================

# Public include directories (visible to users of this library)
target_include_directories(putm_ev_can PUBLIC 
    include 
    dbc/generated
)

if(PUTM_CAN_BACKEND STREQUAL "STM32")
    # --- STM32 Configuration ---
    
    # Define the backend macro for the code
    target_compile_definitions(putm_ev_can PUBLIC PUTM_CAN_BACKEND_STM32)
    
    # Link against STM32CubeMX (assuming provided by parent project)
    target_link_libraries(putm_ev_can PUBLIC stm32cubemx)

    # Resolve paths to STM32 Drivers relative to the project root
    get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
    
    target_include_directories(putm_ev_can PUBLIC
        "${PROJECT_ROOT_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc"
        "${PROJECT_ROOT_DIR}/Drivers/CMSIS/Device/ST/STM32G4xx/Include"
        "${PROJECT_ROOT_DIR}/Drivers/CMSIS/Include"
    )

elseif(PUTM_CAN_BACKEND STREQUAL "ROS2")
    # --- ROS 2 / Linux Configuration ---

    # Define the backend macro for the code
    target_compile_definitions(putm_ev_can PUBLIC PUTM_CAN_BACKEND_ROS2)

    # Find required packages
    find_package(Threads REQUIRED)
    find_package(rclcpp REQUIRED) # Required for logging in can_hal_ros2.cpp
    
    # Link dependencies
    target_link_libraries(putm_ev_can PRIVATE 
        Threads::Threads
        rclcpp::rclcpp
    )
endif()