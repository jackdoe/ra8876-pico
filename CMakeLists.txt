cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico2)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(ra8876_project C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(ra8876_demo
    main.c
    ra8876.c
)

# Enable SPI and Stdlib
target_link_libraries(ra8876_demo
    pico_stdlib
    hardware_spi
    hardware_dma
)

pico_enable_stdio_usb(ra8876_demo 1)
pico_enable_stdio_uart(ra8876_demo 0)

pico_add_extra_outputs(ra8876_demo)
