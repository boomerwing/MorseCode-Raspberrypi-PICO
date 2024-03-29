cmake_minimum_required(VERSION 3.14)

# Should we set app-level debugging?
# NOTE Equivalent of `#define DEBUG 1`
if(${DO_DEBUG})
    add_compile_definitions(DEBUG=1)
    message(STATUS "App-side debugging enabled for ${APP_4_NAME}")
else()
    message(STATUS "App-side debugging disabled for ${APP_4_NAME}")
endif()

# Make project data accessible to compiler
add_compile_definitions(APP_NAME="${APP_4_NAME}")
add_compile_definitions(APP_VERSION="${APP_4_VERSION_NUMBER}")
add_compile_definitions(BUILD_NUM=${BUILD_NUMBER})

# Include app source code file(s)
add_executable(${APP_4_NAME}
    ${APP_4_SRC_DIRECTORY}/main_i2c.c
    ${COMMON_CODE_DIRECTORY}/pcf8575i2c.c
    ${COMMON_CODE_DIRECTORY}/seven_seg.c
    ${COMMON_CODE_DIRECTORY}/ps2.c
    ${COMMON_CODE_DIRECTORY}/kbdpio.pio
)

# Link to built libraries
target_link_libraries(${APP_4_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_pio
    FreeRTOS
    )

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${APP_4_NAME} 1)
pico_enable_stdio_uart(${APP_4_NAME} 0)

# Enable extra build products
pico_add_extra_outputs(${APP_4_NAME})
