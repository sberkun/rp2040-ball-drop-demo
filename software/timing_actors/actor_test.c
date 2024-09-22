#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#include "../actors.h"


#define LOGIC_PIN 1

event_queue_t q;
bool logic = false;
int count = 0;

void my_action(uint64_t logical_time, size_t actor_id, void* arg) {
    // gpio_put(1, false);
    // gpio_put(LOGIC_PIN, false);
    // gpio_put(LOGIC_PIN, logic);
    // logic = !logic;
    // if ((count++) % 10 == 0) {
        uint64_t actual = to_us_since_boot(get_absolute_time());
        printf("AQUEUE actual: %lld, expected: %lld\n", actual, logical_time);
    // }

    schedule_event(&q, logical_time + 1000000, 0, my_action, NULL);
}


// Core 0 Main Code
int main(void){
    // Initialize the pin as an output
    gpio_init(LOGIC_PIN);
    gpio_set_dir(LOGIC_PIN, GPIO_OUT);

    stdio_init_all();
    sleep_ms(5000);

    printf("starting!!\n");

    event_queue_init(&q, 10, 1);
    schedule_event(&q, to_us_since_boot(get_absolute_time()), 0, my_action, NULL);
    work(&q);
}
