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
add_compile_definitions(APP_NAME="${APP_1_NAME}")
add_compile_definitions(APP_VERSION="${APP_1_VERSION_NUMBER}")
add_compile_definitions(BUILD_NUM=${BUILD_NUMBER})

# Include app source code file(s)
add_executable(${APP_1_NAME}
    ${APP_1_SRC_DIRECTORY}/main_i2c.c
    ${COMMON_CODE_DIRECTORY}/pcf8575i2c.c
    ${COMMON_CODE_DIRECTORY}/seven_seg.c
)

# Link to built libraries
target_link_libraries(${APP_1_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c
    FreeRTOS)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${APP_1_NAME} 1)
pico_enable_stdio_uart(${APP_1_NAME} 0)

# Enable extra build products
pico_add_extra_outputs(${APP_1_NAME})
