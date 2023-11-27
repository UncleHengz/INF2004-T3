add_executable(analysis analysis.c)

# pull in common dependencies
target_link_libraries(analysis pico_stdlib)

# enable usb output, enable uart output
pico_enable_stdio_usb(analysis 1)
pico_enable_stdio_uart(analysis 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(analysis)
