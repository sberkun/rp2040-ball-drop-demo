#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#include "../lib/actors.h"



event_queue_t q;



int actor_0_state;

void actor_0_print(uint64_t logical_time, size_t actor_id, void* arg) {
    printf("State: %d\n", actor_0_state);
}

void actor_0_increment(uint64_t logical_time, size_t actor_id, void* arg) {
    actor_0_state += 1;
}

void actor_1_loop(uint64_t logical_time, size_t actor_id, void* arg) {
    schedule_event(&q, logical_time, 0, actor_0_print, NULL);
    schedule_event(&q, logical_time + 1000000, 1, actor_1_loop, NULL);
}

void actor_2_loop(uint64_t logical_time, size_t actor_id, void* arg) {
    schedule_event(&q, logical_time, 0, actor_0_increment, NULL);
    schedule_event(&q, logical_time + 1000000, 2, actor_2_loop, NULL);
}



void core1_entry() {
    work(&q);
}

// Core 0 Main Code
int main(void){
    stdio_init_all();
    sleep_ms(4000);

    printf("starting!!\n");

    uint64_t tt = to_us_since_boot(get_absolute_time());
    event_queue_init(&q, 10, 10);

    schedule_event(&q, tt, 1, actor_1_loop, NULL);
    schedule_event(&q, tt, 2, actor_2_loop, NULL);
    multicore_launch_core1(core1_entry);
    work(&q);
}
