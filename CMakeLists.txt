# Define the name of the library
set(LIB_NAME putm_can)

# Set source and include directories for the library
set(CAN_LIB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)       # Directory for source files
set(CAN_LIB_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)       # Directory for header files
set(GENERATED_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)  # Directory for generated Protobuf files

# Build options for adapters (passed from the main CMake file)
# option(BUILD_STM32_ADAPTER "Build STM32 adapter" ON)    # Commented out, as it’s passed from the main file
# option(BUILD_ROS2_ADAPTER "Build ROS2 adapter" OFF)     # Commented out, as it’s passed from the main file

# Add Protobuf support
find_package(Protobuf REQUIRED)                            # Check if Protobuf is available
if (NOT Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf not found. Please install libprotobuf-dev and protobuf-compiler.")  # Error if Protobuf is missing
endif()

# Set paths for Protobuf files (relative to the main project directory)
set(PROTO_SRC_DIR "${CMAKE_SOURCE_DIR}/proto")            # Directory containing .proto files
file(MAKE_DIRECTORY ${GENERATED_PROTO_DIR})               # Create directory for generated files if it doesn’t exist

# Find all .proto files
file(GLOB PROTO_FILES "${PROTO_SRC_DIR}/*.proto")         # Collect all .proto files in the proto directory

# Generate .pb.h and .pb.cc files from .proto files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})  # Generate Protobuf source and header files

# Create a library for generated Protobuf files
add_library(putm_can_proto ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(putm_can_proto PUBLIC
    ${GENERATED_PROTO_DIR}                                # Include directory for generated Protobuf headers
    ${Protobuf_INCLUDE_DIRS}                              # Include Protobuf library headers
)
target_link_libraries(putm_can_proto PUBLIC ${Protobuf_LIBRARIES})  # Link against Protobuf libraries

# Create the core library
add_library(${LIB_NAME}
    ${CAN_LIB_SRC_DIR}/message_handler.cpp                # Add core source files here
    # Add other core files, e.g., can_interface.cpp, if they exist
)
target_include_directories(${LIB_NAME} PUBLIC
    ${CAN_LIB_INC_DIR}                                    # Include directory for library headers
    ${GENERATED_PROTO_DIR}                                # Include directory for generated Protobuf headers
    ${Protobuf_INCLUDE_DIRS}                              # Include Protobuf library headers
)
target_link_libraries(${LIB_NAME} PUBLIC putm_can_proto)   # Link core library with Protobuf library

# Conditionally add STM32 adapter
if(BUILD_STM32_ADAPTER)
    message(STATUS "Building STM32 adapter")              # Inform user that STM32 adapter is being built
    add_library(stm32_can
        ${CAN_LIB_SRC_DIR}/stm32_can.cpp                  # Add STM32 adapter source file
    )
    target_include_directories(stm32_can PUBLIC
        ${CAN_LIB_INC_DIR}                                # Include directory for library headers
        ${GENERATED_PROTO_DIR}                            # Include directory for generated Protobuf headers
        ${Protobuf_INCLUDE_DIRS}                          # Include Protobuf library headers
    )
    target_link_libraries(stm32_can PUBLIC ${LIB_NAME} stm32cubemx)  # Link STM32 adapter with core and CubeMX
    # Export the library to the main project
    set(STATIC_LIBS ${STATIC_LIBS} stm32_can PARENT_SCOPE)
endif()

# Conditionally add ROS2 adapter
if(BUILD_ROS2_ADAPTER)
    message(STATUS "Building ROS2 adapter")              # Inform user that ROS2 adapter is being built
    find_package(rclcpp REQUIRED)                        # Find rclcpp package for ROS2
    find_package(can_msgs REQUIRED)                      # Find can_msgs package for ROS2
    add_library(ros2_can
        ${CAN_LIB_SRC_DIR}/ros2_can.cpp                  # Add ROS2 adapter source file
    )
    target_include_directories(ros2_can PUBLIC
        ${CAN_LIB_INC_DIR}                                # Include directory for library headers
        ${GENERATED_PROTO_DIR}                            # Include directory for generated Protobuf headers
        ${Protobuf_INCLUDE_DIRS}                          # Include Protobuf library headers
    )
    target_link_libraries(ros2_can PUBLIC
        ${LIB_NAME}                                       # Link with core library
        rclcpp::rclcpp                                    # Link with rclcpp library
        can_msgs::can_msgs                                # Link with can_msgs library
    )
    # Export the library to the main project
    set(STATIC_LIBS ${STATIC_LIBS} ros2_can PARENT_SCOPE)
endif()

# Export the core library to the main project
set(STATIC_LIBS ${LIB_NAME} ${STATIC_LIBS} PARENT_SCOPE)   # Pass core and adapter libraries to the parent scope

# Add include paths for the main project
target_include_directories(${LIB_NAME} INTERFACE
    ${CAN_LIB_INC_DIR}                                    # Export include directory for main project
    ${GENERATED_PROTO_DIR}                                # Export generated Protobuf include directory
)