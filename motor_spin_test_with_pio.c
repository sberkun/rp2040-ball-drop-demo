#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "demo_pins.h"
#include "motor_driver.pio.h"


int target_step_time(bool fast) {
    return fast ? 40 : 2500; 
    // 2500 = 8 secs per rotation 
    // 312.5 = 1 second per rotation
}

// Core 0 Main Code
int main(void){
    stdio_init_all();

    initialize_demo_pins();
    mdrive_all_setup(STEP_PIN);
    mdrive_go(4000);
    
    int arcade_button_saved = 1;
    uint64_t arcade_button_last_pressed = 0;
    uint64_t toggle_time = to_us_since_boot(get_absolute_time());
    uint64_t actual_step_with_mul = 4000 * ACCEL_MUL_PIO;
    bool fast = false;

    while (true) {
        uint64_t time = to_us_since_boot(get_absolute_time());
        

        // check button - 1s debounce
        int arcade_button_new = gpio_get(ARCADE_BUTTON_PIN);
        if (arcade_button_new != arcade_button_saved && time - arcade_button_last_pressed > 1000000) {
            fast = !fast;
            printf("button! %lld\n", (actual_step_with_mul / ACCEL_MUL_PIO));
            arcade_button_last_pressed = time;
        }
        arcade_button_saved = arcade_button_new;

        // adjust speed based on target speed
        uint64_t target_st = target_step_time(fast);
        uint64_t actual_st = actual_step_with_mul / ACCEL_MUL_PIO;
        if (actual_st > target_st) {
            actual_step_with_mul -= actual_st;
        }
        if (actual_st < target_st) {
            actual_step_with_mul += actual_st;
        }

        // setup for step; do four steps
        mdrive_go(actual_step_with_mul / ACCEL_MUL_PIO);
        mdrive_go(actual_step_with_mul / ACCEL_MUL_PIO);
        toggle_time += 4 * (actual_step_with_mul / ACCEL_MUL_PIO);
        sleep_until(from_us_since_boot(toggle_time));
    }
}
