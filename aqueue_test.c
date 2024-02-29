#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#include "action_queue.h"


action_queue_t q;

void action(uint64_t logical_time) {
    uint64_t actual = to_us_since_boot(get_absolute_time());
    printf("AQUEUE actual: %lld, expected: %lld\n", actual, logical_time);

    schedule_action(&q, action, logical_time + 10000);
}


// Core 0 Main Code
int main(void){
    stdio_init_all();
    sleep_ms(5000);
    action_queue_init(&q, 10);
    schedule_action(&q, action, to_us_since_boot(get_absolute_time()));
    while (true) {
        do_next_action(&q);
    }
}
