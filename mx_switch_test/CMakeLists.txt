add_executable(mx_switch_test main.c)

target_link_libraries(mx_switch_test
        pico_stdlib
        hardware_gpio
)

pico_enable_stdio_usb(mx_switch_test 1)
pico_enable_stdio_uart(mx_switch_test 0)

pico_add_extra_outputs(mx_switch_test)