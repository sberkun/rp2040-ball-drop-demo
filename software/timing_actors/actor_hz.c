#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#include "../lib/actors.h"


#define LOGIC_PIN_A 2
#define LOGIC_PIN_B 3
#define INTERVAL 100

event_queue_t q;
int count = 0;

int sumsq(int total, int n);

static inline void bench() {
    int total = 0;
    for (int a = 0; a < 210; a++) {
        total = sumsq(total, 5);
    }
}


void my_action_a(uint64_t logical_time, size_t actor_id, void* arg) {
    gpio_put(LOGIC_PIN_A, true);
    bench();
    gpio_put(LOGIC_PIN_A, false);

    schedule_event(&q, logical_time + INTERVAL, actor_id, my_action_a, NULL);
}

void my_action_b(uint64_t logical_time, size_t actor_id, void* arg) {
    gpio_put(LOGIC_PIN_B, true);
    bench();
    gpio_put(LOGIC_PIN_B, false);

    schedule_event(&q, logical_time + INTERVAL, actor_id, my_action_b, NULL);
}



void core1_entry() {
    work(&q);
}

int main(void){
    // Initialize the pins as an output
    gpio_init(LOGIC_PIN_A);
    gpio_init(LOGIC_PIN_B);
    gpio_set_dir(LOGIC_PIN_A, GPIO_OUT);
    gpio_set_dir(LOGIC_PIN_B, GPIO_OUT);

    event_queue_init(&q, 10, 10);
    uint64_t tt = to_us_since_boot(get_absolute_time());
    schedule_event(&q, tt, 0, my_action_a, NULL);
    schedule_event(&q, tt + (INTERVAL / 2), 1, my_action_b, NULL);
    multicore_launch_core1(core1_entry);
    work(&q);
}
