add_executable(hid_usb_test
        main.c
        usb_descriptors.c
)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(hid_usb_test PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(hid_usb_test
        pico_stdlib
        hardware_gpio
        tinyusb_device
        tinyusb_board
)

pico_enable_stdio_usb(hid_usb_test 0)
pico_enable_stdio_uart(hid_usb_test 0)

pico_add_extra_outputs(hid_usb_test)