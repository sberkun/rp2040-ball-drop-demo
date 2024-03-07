cmake_minimum_required(VERSION 3.12)

include(set_env.cmake)
include(pico_sdk_import.cmake)

project(ball-drop-spinny-thingy)

pico_sdk_init()

add_executable(spinny
        # all_together.c
        # motor_spin_test.c
        # magnet_photogate_test.c
        timing_tests/aqueue_test.c
        action_queue.c
)

target_link_libraries(spinny
        pico_multicore
        pico_stdlib
        hardware_adc
)

pico_enable_stdio_usb(spinny 1)
pico_enable_stdio_uart(spinny 0)

pico_add_extra_outputs(spinny)
