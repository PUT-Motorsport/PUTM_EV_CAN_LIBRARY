set(PUTM_EV_CAN_LIBRARY_INC 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/CanHeaders
)

# message(PUTM_EV_CAN_LIBRARY_INC=${PUTM_EV_CAN_LIBRARY_INC})
set(PUTM_EV_CAN_LIBRARY_VER "?.?")

add_library(PUTM_EV_CAN_LIBRARY INTERFACE)
# target_include_directories(PUTM_EV_CAN_LIBRARY INTERFACE ${MX_Include_Dirs})
target_include_directories(PUTM_EV_CAN_LIBRARY INTERFACE ${PUTM_EV_CAN_LIBRARY_INC})

target_link_libraries(PUTM_EV_CAN_LIBRARY)
target_link_libraries(${CMAKE_PROJECT_NAME} PUTM_EV_CAN_LIBRARY)

message("PUTM_EV_CAN_LIBRARY version: " ${PUTM_EV_CAN_LIBRARY_VER} " included in the project")