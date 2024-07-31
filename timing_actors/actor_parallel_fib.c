#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#include "../actors.h"


uint64_t current_time_us() {
    return to_us_since_boot(get_absolute_time());
}

int fib(int n);

void __noinline bench() {
    uint64_t t = current_time_us();
    int r = fib(25);
    t = current_time_us() - t;
    printf("Timing fib %d %d %lld\n", get_core_num(), r, t);
}



event_queue_t q;

void my_action(uint64_t logical_time, size_t actor_id, void* arg) {
    bench();
    schedule_event(&q, logical_time + 1000000, actor_id, my_action, NULL);
}

void core1_entry() {
    work(&q);
}

// Core 0 Main Code
int main(void){
    stdio_init_all();

    event_queue_init(&q, 10, 2);
    uint64_t tt = to_us_since_boot(get_absolute_time());
    schedule_event(&q, tt, 0, my_action, NULL);
    schedule_event(&q, tt, 1, my_action, NULL);
    multicore_launch_core1(core1_entry);
    work(&q);
}
