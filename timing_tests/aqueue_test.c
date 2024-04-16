#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#include "../action_queue.h"


#define LOGIC_PIN 1

action_queue_t q;
bool logic = false;
int count = 0;

void action(uint64_t logical_time) {
    // gpio_put(1, false);
    // gpio_put(LOGIC_PIN, false);
    // gpio_put(LOGIC_PIN, logic);
    // logic = !logic;
    if ((count++) % 10 == 0) {
        uint64_t actual = to_us_since_boot(get_absolute_time());
        printf("AQUEUE actual: %lld, expected: %lld\n", actual, logical_time);
    }

    schedule_action(&q, action, logical_time + 100000);
}


// Core 0 Main Code
int main(void){
    // Initialize the pin as an output
    gpio_init(LOGIC_PIN);
    gpio_set_dir(LOGIC_PIN, GPIO_OUT);

    stdio_init_all();
    sleep_ms(5000);

    action_queue_init(&q, 10);
    schedule_action(&q, action, to_us_since_boot(get_absolute_time()));
    while (true) {
        do_next_action(&q);
    }
}
