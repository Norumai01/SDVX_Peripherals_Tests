cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(controller_subtests C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(mx_switch_test)
add_subdirectory(encoder_test)
add_subdirectory(button_encoder_test)
add_subdirectory(hid_usb_test)
