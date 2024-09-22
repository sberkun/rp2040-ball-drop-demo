cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(ball-drop-spinny-thingy)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_BUILD_TYPE Release)

pico_sdk_init()

add_executable(spinny
        # timing_tests/parallel_int_math.c
        # timing_tests/parallel_float_math.c
        # timing_tests/parallel_matmul.c
        # timing_tests/parallel_shuffle.c
        timing_tests/parallel_fib.c
        timing_tests/sep_fib.s
)

# pico_generate_pio_header(spinny ${CMAKE_CURRENT_LIST_DIR}/motor_driver.pio)
# pico_set_linker_script(spinny /home/sberkun/Documents/research/picostuff/pico-sdk/src/rp2_common/pico_standard_link/memmap_blocked_ram.ld)


target_link_libraries(spinny
        pico_multicore
        pico_stdlib
        hardware_adc
        hardware_pio
)

pico_enable_stdio_usb(spinny 1)
pico_enable_stdio_uart(spinny 0)

pico_add_extra_outputs(spinny)
