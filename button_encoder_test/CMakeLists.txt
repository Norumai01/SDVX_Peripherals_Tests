add_executable(button_encoder_test main.c)

target_link_libraries(button_encoder_test
        pico_stdlib
        hardware_gpio
)

pico_enable_stdio_usb(button_encoder_test 1)
pico_enable_stdio_uart(button_encoder_test 0)

pico_add_extra_outputs(button_encoder_test)