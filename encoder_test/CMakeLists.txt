add_executable(encoder_test main.c)

target_link_libraries(encoder_test
        pico_stdlib
        hardware_gpio
)

pico_enable_stdio_usb(encoder_test 1)
pico_enable_stdio_uart(encoder_test 0)

pico_add_extra_outputs(encoder_test)