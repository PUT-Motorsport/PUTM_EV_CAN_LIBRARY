# Core/Inc/PUTM_EV_CAN_LIBRARY/CMakeLists.txt
#
# @brief CMake configuration for PUTM CAN library using DBC + cantools.
#        Generates .c/.h files from *.dbc and builds the core library.
#        Protobuf is completely removed.

cmake_minimum_required(VERSION 3.20)
set(LIB_NAME putm_can)

# === PATHS ===
set(CAN_LIB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CAN_LIB_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(GENERATED_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/generated)
set(DBC_DIR         ${CMAKE_CURRENT_SOURCE_DIR}/dbc)

# Ensure output directory exists
file(MAKE_DIRECTORY ${GENERATED_DIR})

# === FIND PYTHON + CANTOOLS ===
find_package(Python3 REQUIRED COMPONENTS Interpreter)
if(NOT Python3_Interpreter_FOUND)
    message(FATAL_ERROR "Python3 interpreter not found. Install Python and cantools: pip install cantools")
endif()

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import cantools, sys; print(cantools.__version__)"
    OUTPUT_VARIABLE CANTOOLS_VERSION
    RESULT_VARIABLE CANTOOLS_RESULT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(CANTOOLS_RESULT)
    message(FATAL_ERROR "cantools not installed. Run: pip install cantools")
else()
    message(STATUS "Found cantools version ${CANTOOLS_VERSION}")
endif()

# === FIND ALL .dbc FILES ===
file(GLOB DBC_FILES "${DBC_DIR}/*.dbc")
if(NOT DBC_FILES)
    message(WARNING "No .dbc files found in ${DBC_DIR}")
endif()

# === GENERATE .c/.h FROM DBC ===
set(GENERATED_SOURCES "")
set(GENERATED_HEADERS "")

foreach(DBC_FILE ${DBC_FILES})
    get_filename_component(DBC_NAME ${DBC_FILE} NAME_WE)
    set(GEN_C "${GENERATED_DIR}/${DBC_NAME}.c")
    set(GEN_H "${GENERATED_DIR}/${DBC_NAME}.h")

    add_custom_command(
        OUTPUT ${GEN_C} ${GEN_H}
        COMMAND ${Python3_EXECUTABLE} -m cantools generate_c_source
                ${DBC_FILE}
                -o ${GENERATED_DIR}
        DEPENDS ${DBC_FILE}
        COMMENT "Generating ${DBC_NAME}.c/.h from DBC"
        VERBATIM
    )

    list(APPEND GENERATED_SOURCES ${GEN_C})
    list(APPEND GENERATED_HEADERS ${GEN_H})
endforeach()

# === CORE LIBRARY ===
add_library(${LIB_NAME}
    ${CAN_LIB_SRC_DIR}/can_interface.cpp
    ${CAN_LIB_SRC_DIR}/message_handler.cpp
    ${GENERATED_SOURCES}
)

target_include_directories(${LIB_NAME} PUBLIC
    ${CAN_LIB_INC_DIR}
    ${GENERATED_DIR}
)

# === STM32 ADAPTER ===
if(BUILD_STM32_ADAPTER)
    message(STATUS "Building STM32 CAN adapter")
    add_library(stm32_can
        ${CAN_LIB_SRC_DIR}/can_hal_stm32.cpp
    )
    target_include_directories(stm32_can PUBLIC
        ${CAN_LIB_INC_DIR}
        ${GENERATED_DIR}
    )
    target_link_libraries(stm32_can PUBLIC ${LIB_NAME} stm32cubemx)
    set(STATIC_LIBS ${STATIC_LIBS} stm32_can PARENT_SCOPE)
    message(STATUS "STM32 adapter built")
endif()

# === ROS2 ADAPTER ===
if(BUILD_ROS2_ADAPTER)
    message(STATUS "Building ROS2 CAN adapter")
    find_package(rclcpp REQUIRED)
    find_package(can_msgs REQUIRED)
    add_library(ros2_can
        ${CAN_LIB_SRC_DIR}/can_ros2.cpp
    )
    target_include_directories(ros2_can PUBLIC
        ${CAN_LIB_INC_DIR}
        ${GENERATED_DIR}
    )
    target_link_libraries(ros2_can PUBLIC
        ${LIB_NAME}
        rclcpp::rclcpp
        can_msgs::can_msgs
    )
    set(STATIC_LIBS ${STATIC_LIBS} ros2_can PARENT_SCOPE)
    message(STATUS "ROS2 adapter built")
endif()

# Export to parent
set(STATIC_LIBS ${LIB_NAME} ${STATIC_LIBS} PARENT_SCOPE)