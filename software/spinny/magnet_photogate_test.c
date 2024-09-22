#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"

#include "demo_pins.h"



// Core 0 Main Code
int main(void){
    stdio_init_all();

    initialize_demo_pins();

    int sense_1_saved = true;
    int sense_2_saved = true;
    int arcade_button_saved = false;
    int last_event_time = 0;

    while (true) {
        uint64_t time = to_us_since_boot(get_absolute_time());
        int sense_1_new = gpio_get(SENSE_1_PIN);
        int sense_2_new = gpio_get(SENSE_2_PIN);
        int arcade_button_new = gpio_get(ARCADE_BUTTON_PIN);

        if (sense_1_new != sense_1_saved) {
            printf("sense1: %d %lld\n", sense_1_new, (time - last_event_time));
            last_event_time = time;
        }
        if (sense_2_new != sense_2_saved) {
            printf("sense2: %d %lld\n", sense_2_new, (time - last_event_time));
            last_event_time = time;
        }
        if (arcade_button_new != arcade_button_saved) {
            printf("ab: %d %lld\n", arcade_button_new, (time - last_event_time));
            gpio_put(MAGNET_PIN, !arcade_button_new);
            last_event_time = time;
        }

        sense_1_saved = sense_1_new;
        sense_2_saved = sense_2_new;
        arcade_button_saved = arcade_button_new;

        sleep_us(50);
    }
}
