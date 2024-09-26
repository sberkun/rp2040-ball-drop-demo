#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#include "../lib/actors.h"


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
        printf("AQUEUE core: %d, actual: %lld, expected: %lld\n", get_core_num(), actual, logical_time);
    // }

    schedule_event(&q, logical_time + 1000000, actor_id, my_action, NULL);
}


// void periodic_printer(uint64_t logical_time, size_t actor_id, void* arg) {
//    printf("Hello from actor %d!\n", actor_id);
//    schedule_event(&global_event_queue, logical_time + 1000000, actor_id, periodic_printer, NULL);
// }



void core1_entry() {
    work(&q);
}

// Core 0 Main Code
int main(void){
    // Initialize the pin as an output
    gpio_init(LOGIC_PIN);
    gpio_set_dir(LOGIC_PIN, GPIO_OUT);

    stdio_init_all();
    sleep_ms(4000);

    printf("starting!! %d %d %d\n", sizeof(event_queue_t), sizeof(event_t), sizeof(bool));

    void* cheese = malloc(1024);
    printf("bare-metal printer start: %p\n", cheese);

    uint64_t tt = to_us_since_boot(get_absolute_time());
    int init_success = event_queue_init(&q, 100, 100);
    printf("did init: %d\n", init_success);

    schedule_event(&q, tt, 0, my_action, NULL);
    schedule_event(&q, tt + 100000, 1, my_action, NULL);
    multicore_launch_core1(core1_entry);
    work(&q);
}
