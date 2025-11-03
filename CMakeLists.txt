# PUTM_EV_CAN_LIBRARY/CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

project(putm_ev_can VERSION 1.0.0 LANGUAGES C CXX)

# Ustawienia kompilatora
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Python3 COMPONENTS Interpreter QUIET)

if(Python3_Interpreter_FOUND)
    # Sprawdź czy plik DBC istnieje
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dbc/PUTM_CAN_1.dbc)
        execute_process(
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/dbc/setup.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbc
            RESULT_VARIABLE CODE_GEN_RESULT
            OUTPUT_QUIET
            ERROR_QUIET
        )
        if(CODE_GEN_RESULT EQUAL 0)
            message(STATUS "DBC code generation successful")
        else()
            message(WARNING "DBC code generation failed")
        endif()
    else()
        message(WARNING "DBC file not found, creating minimal definitions")
        # Utwórz minimalne pliki jeśli DBC nie istnieje
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/dbc/generated/can_message_defs.h
            "#ifndef CAN_MESSAGE_DEFS_H\n#define CAN_MESSAGE_DEFS_H\n\n// Minimal definitions\n#include <stdint.h>\n\n#endif\n")
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/dbc/generated/can_message_defs.c "// Minimal implementation\n")
    endif()
else()
    message(WARNING "🐍 Python3 not found - using minimal CAN definitions")
endif()

# AUTOMATYCZNE WYSZUKIWANIE PLIKÓW ŹRÓDŁOWYCH
file(GLOB_RECURSE CAN_SOURCES 
    "src/*.cpp"
    "src/*.c"
    "dbc/generated/*.c"
)

# Jeśli nie znaleziono plików, utwórz minimalną bibliotekę
if(NOT CAN_SOURCES)
    message(STATUS "📁 No source files found, creating minimal library")
    set(CAN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/minimal.cpp)
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/src/minimal.cpp "// Minimal CAN library\nnamespace putm_ev_can { int placeholder = 1; }\n")
endif()

# UTWÓRZ BIBLIOTEKĘ
add_library(putm_ev_can STATIC ${CAN_SOURCES})

# AUTOMATYCZNE DOŁĄCZANIE NAGŁÓWKÓW
target_include_directories(putm_ev_can PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dbc/generated>
)

target_include_directories(putm_ev_can PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)


message(STATUS "PUTM_EV_CAN_LIBRARY configured - just add 'putm_ev_can' to target_link_libraries")