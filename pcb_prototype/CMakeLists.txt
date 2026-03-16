add_executable(pcb_prototype
        main.c
        usb_descriptors.c
)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(pcb_prototype PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(pcb_prototype
        pico_stdlib
        hardware_gpio
        tinyusb_device
        tinyusb_board
)

pico_enable_stdio_usb(pcb_prototype 0)
pico_enable_stdio_uart(pcb_prototype 0)

pico_add_extra_outputs(pcb_prototype)