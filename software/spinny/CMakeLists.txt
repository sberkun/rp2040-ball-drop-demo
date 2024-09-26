cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(ball-drop-spinny-thingy)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_BUILD_TYPE Release)

pico_sdk_init()

add_executable(spinny
        # all_together_actors.c
        # all_together.c
        # motor_spin_test_with_pio.c
        magnet_photogate_test.c
        # motor_spin_test.c
        ../lib/actors.c
)

# pico_generate_pio_header(spinny ${CMAKE_CURRENT_LIST_DIR}/motor_driver.pio)
# pico_set_linker_script(spinny ${CMAKE_CURRENT_LIST_DIR}/../../pico-sdk/src/rp2_common/pico_standard_link/memmap_blocked_ram.ld)


target_link_libraries(spinny
        pico_multicore
        pico_stdlib
        hardware_adc
        hardware_pio
)

pico_enable_stdio_usb(spinny 1)
pico_enable_stdio_uart(spinny 0)

pico_add_extra_outputs(spinny)
