#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"

#include "demo_pins.h"




// Core 0 Main Code
int main(void){
    stdio_init_all();

    initialize_demo_pins();

    sleep_ms(5000);

    
    int arcade_button_saved = 1;
    uint64_t arcade_button_last_pressed = 0;

    uint64_t toggle_time = to_us_since_boot(get_absolute_time());
    uint64_t actual_step_with_mul = 2500 * ACCEL_MUL;
    int motor_saved = 0;
    int motor_position = 0;

    bool sense_1_saved = true;
    bool sense_2_saved = true;
    
    uint64_t target_st = 3000;
    uint64_t expected_tunnel_time = 0; // state 3 only
    int countdown = 0;                 // state 3 only
    int target_pos_to_drop = 0;        // state 2 only
    int state = 0;
    // state = 0 - pre-arm, motor slowly spinning. button press ->
    // state = 1 - armed, motor spinning fast. button press ->
    // state = 2 - waiting for correct position. when reached, unarm
    // state = 3 - falling! waiting for countdown

    while (true) {
        uint64_t time = to_us_since_boot(get_absolute_time());
        
        // check button - 1s debounce
        int arcade_button_new = gpio_get(ARCADE_BUTTON_PIN);
        if (arcade_button_new != arcade_button_saved && time - arcade_button_last_pressed > 1000000) {
            printf("button! state:%d, %lld\n",  state, (actual_step_with_mul / ACCEL_MUL));
            arcade_button_last_pressed = time;

            // state machine logic
            if (state == 0) {
                state = 1;
                gpio_put(MAGNET_PIN, true);
                target_st = 1500;
            } else if (state == 1) {
                state = 2;
                // calculate drop position
                int drop_steps = (US_AFTER_E0 / target_st) % STEPS_PER_HALF_ROT;
                target_pos_to_drop = (STEPS_PER_HALF_ROT - drop_steps) % STEPS_PER_HALF_ROT;
                printf("drop position: %d %d %d\n", US_AFTER_E0, drop_steps, target_pos_to_drop);
            }
        }
        arcade_button_saved = arcade_button_new;

        // state machine timing logic
        if (state == 2 && motor_position == target_pos_to_drop) {
            state = 3;
            gpio_put(MAGNET_PIN, false);
            expected_tunnel_time = time + US_AFTER_E0; 
            countdown = US_AFTER_E0 / target_st;
        }
        if (state == 3 && time > expected_tunnel_time + 500000) {
            state = 0;
            target_st = 3000;
        }

        // speed adjustment during fall
        if (state == 3) {
            // check sensors
            bool sense_1_new = gpio_get(SENSE_1_PIN);
            bool sense_2_new = gpio_get(SENSE_2_PIN);
            if (!sense_1_new && sense_1_saved) { // event 1
                printf("event 1: %lld -> %lld\n", expected_tunnel_time, time + US_AFTER_E1);
                expected_tunnel_time = time + US_AFTER_E1;
            }
            if (sense_1_new && !sense_1_saved) { // event 2
                printf("event 2: %lld -> %lld\n", expected_tunnel_time, time + US_AFTER_E2);
                expected_tunnel_time = time + US_AFTER_E2;
            }
            if (!sense_2_new && sense_2_saved) { // event 3
                printf("event 3: %lld -> %lld\n", expected_tunnel_time, time + US_AFTER_E3);
                expected_tunnel_time = time + US_AFTER_E3;
            }
            if (sense_2_new && !sense_2_saved) { // event 4
                printf("event 4: %lld -> %lld\n", expected_tunnel_time, time + US_AFTER_E4);
                expected_tunnel_time = time + US_AFTER_E3;
            }
            sense_1_saved = sense_1_new;
            sense_2_saved = sense_2_new;

            // adjust target speed
            // TODO: uncomment once the sensors actually work
            // uint64_t actual_st = actual_step_with_mul / ACCEL_MUL;
            // uint64_t disk_tunnel_time = time + (actual_st * countdown);
            // if (disk_tunnel_time < expected_tunnel_time) {
            //     // disk will arrive first: slow down!
            //     target_st = 60;
            // } else {
            //     // ball will arrive first: speed up!
            //     target_st = 40;
            // }
            // countdown -= 1;
        }


        // adjust speed based on target speed
        uint64_t actual_st = actual_step_with_mul / ACCEL_MUL;
        if (actual_st > target_st) {
            actual_step_with_mul -= actual_st;
        }
        if (actual_st < target_st) {
            actual_step_with_mul += actual_st;
        }

        // setup for step; do one step
        toggle_time += (actual_step_with_mul / ACCEL_MUL);
        motor_saved = !motor_saved;
        motor_position += 1;
        if (motor_position == STEPS_PER_HALF_ROT) {
            motor_position = 0;
        }
        sleep_until(from_us_since_boot(toggle_time));
        gpio_put(STEP_PIN, motor_saved);
    }
}
