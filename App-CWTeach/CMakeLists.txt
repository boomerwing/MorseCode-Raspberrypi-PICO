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
add_compile_definitions(APP_NAME="${APP_6_NAME}")
add_compile_definitions(APP_VERSION="${APP_6_VERSION_NUMBER}")
add_compile_definitions(BUILD_NUM=${BUILD_NUMBER})

# Include app source code file(s)
add_executable(${APP_6_NAME}
    ${APP_6_SRC_DIRECTORY}/main_pio.c
)

# Link to built libraries
target_link_libraries(${APP_6_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_pio
    hardware_clocks
    FreeRTOS)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${APP_6_NAME} 1)
pico_enable_stdio_uart(${APP_6_NAME} 0)

# Enable extra build products
pico_add_extra_outputs(${APP_6_NAME})
