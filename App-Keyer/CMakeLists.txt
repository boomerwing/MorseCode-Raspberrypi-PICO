cmake_minimum_required(VERSION 3.14)

# Should we set app-level debugging?
# NOTE Equivalent of `#define DEBUG 1`
if(${DO_DEBUG})
    add_compile_definitions(DEBUG=1)
    message(STATUS "App-side debugging set")
else()
    message(STATUS "App-side debugging disabled")
endif()

# Make project data accessible to compiler
add_compile_definitions(APP_NAME="${APP_3_NAME}")
add_compile_definitions(APP_VERSION="${APP_3_VERSION_NUMBER}")
add_compile_definitions(BUILD_NUM=${BUILD_NUMBER})

# Include app source code file(s)
add_executable(${APP_3_NAME}
#    ${APP_3_SRC_DIRECTORY}/main_i2c_Ring_B.c
#    ${APP_3_SRC_DIRECTORY}/main_i2c_Ring_C.c
#    ${APP_3_SRC_DIRECTORY}/main_i2c_QRing.c
    ${APP_3_SRC_DIRECTORY}/main_i2c_ShowRing.c
#    ${APP_3_SRC_DIRECTORY}/main_i2c_Ring.c
    ${COMMON_CODE_DIRECTORY}/PCF8575_i2c/pcf8575i2c.c
    ${COMMON_CODE_DIRECTORY}/Seven_Seg_i2c/seven_seg.c
    ${COMMON_CODE_DIRECTORY}/ps2/ps2.c
)

# Link to built libraries
target_link_libraries(${APP_3_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_adc
    hardware_pio
    hardware_clocks
    FreeRTOS)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${APP_3_NAME} 1)
pico_enable_stdio_uart(${APP_3_NAME} 0)

# Enable extra build products
pico_add_extra_outputs(${APP_3_NAME})
