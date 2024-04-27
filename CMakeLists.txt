cmake_minimum_required(VERSION 3.12)

include(set_env.cmake)
include(pico_sdk_import.cmake)

project(ball-drop-spinny-thingy)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_BUILD_TYPE Release)

pico_sdk_init()

add_executable(spinny
        all_together_actors.c
        # all_together.c
        # motor_spin_test_with_pio.c
        # magnet_photogate_test.c
        # timing_tests/aqueue_test.c
        # timing_actors/actor_test.c
        # timing_actors/actor_test_duo.c
        # timing_actors/actor_blink.c
        # timing_actors/actor_race.c
        actors.c
        # timing_tests/parallel_int_math.c
        # timing_tests/parallel_float_math.c
        # timing_tests/parallel_matmul.c
        # timing_tests/parallel_shuffle.c
        # timing_tests/parallel_fib.c
        # timing_tests/sep_fib.s
)

# pico_generate_pio_header(spinny ${CMAKE_CURRENT_LIST_DIR}/motor_driver.pio)

target_link_libraries(spinny
        pico_multicore
        pico_stdlib
        hardware_adc
        hardware_pio
)

pico_enable_stdio_usb(spinny 1)
pico_enable_stdio_uart(spinny 0)

pico_add_extra_outputs(spinny)
