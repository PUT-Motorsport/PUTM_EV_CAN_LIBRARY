cmake_minimum_required(VERSION 3.16)

project(PUTM_EV_CAN_LIBRARY LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# =========================================================
# --- Backend Configuration ---
# =========================================================
# Select the backend platform. Default is STM32.
if(NOT DEFINED PUTM_CAN_BACKEND)
    set(PUTM_CAN_BACKEND STM32)
endif()

# Define preprocessor macros based on the selected backend.
# These macros can be checked in C/C++ code using #ifdef.
if(PUTM_CAN_BACKEND STREQUAL "STM32")
    add_compile_definitions(PUTM_CAN_BACKEND_STM32)
elseif(PUTM_CAN_BACKEND STREQUAL "ROS2")
    add_compile_definitions(PUTM_CAN_BACKEND_ROS2)
endif()

# =========================================================
# --- Python Integration & Code Generation ---
# =========================================================

# 1. Locate the Python interpreter.
# This is safer than hardcoding "python" or "python3".
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# 2. Define paths for the script and its working directory.
set(CAN_GEN_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/dbc/setup.py")
set(CAN_GEN_WORKDIR "${CMAKE_CURRENT_SOURCE_DIR}/dbc")

# 3. [CONFIGURE-TIME] Initial Code Generation.
# This runs immediately when CMake configures the project (before compilation).
# CRITICAL: We must generate the files HERE so that 'file(GLOB_RECURSE)' 
# (called below) can actually find the .c/.h files. If we skip this, 
# the library will be empty on the first build.
execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CAN_GEN_SCRIPT}"
    WORKING_DIRECTORY "${CAN_GEN_WORKDIR}"
    RESULT_VARIABLE GEN_RESULT
)

if(NOT GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "Error: CAN code generation failed during configuration.")
endif()

# 4. [BUILD-TIME] Continuous Update Check.
# This adds a custom target that runs every time you click 'Build'.
# It executes the Python script to check if DBC files have changed.
# Since the script handles MD5 caching, this step is very fast if no changes occurred.
add_custom_target(
    GenerateCanCode ALL
    COMMAND ${Python3_EXECUTABLE} "${CAN_GEN_SCRIPT}"
    WORKING_DIRECTORY "${CAN_GEN_WORKDIR}"
    COMMENT "Checking and regenerating DBC files (Python)..."
)

# =========================================================
# --- Source Files & Library Definition ---
# =========================================================

# Find all source files.
# Thanks to step #3, this will now include the generated files in 'dbc/generated'.
file(GLOB_RECURSE CAN_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dbc/generated/*.c"
)

# Create the static library from found sources.
add_library(putm_ev_can STATIC ${CAN_SOURCES})

# Enforce build order: The library depends on the generator target.
# This ensures 'GenerateCanCode' runs before the library compiles.
add_dependencies(putm_ev_can GenerateCanCode)

# Link against STM32CubeMX (assuming it is provided by the parent project).
target_link_libraries(putm_ev_can PUBLIC stm32cubemx)

# =========================================================
# --- Paths & Include Directories ---
# =========================================================

# Resolve absolute paths to external directories (Drivers, Core/Inc).
get_filename_component(MAIN_H_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

# Configure include paths for the library.
target_include_directories(putm_ev_can PUBLIC
    include             # Library public headers
    dbc/generated       # Generated CAN headers
    "${MAIN_H_DIR}"     # Path to main.h
    "${PROJECT_ROOT_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc"
    "${PROJECT_ROOT_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy"
    "${PROJECT_ROOT_DIR}/Drivers/CMSIS/Device/ST/STM32G4xx/Include"
    "${PROJECT_ROOT_DIR}/Drivers/CMSIS/Include"
)