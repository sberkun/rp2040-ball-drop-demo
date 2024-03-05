#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"


uint64_t current_time_us() {
    return to_us_since_boot(get_absolute_time());
}

/*
void sleep_until_us(uint64_t us) {
    absolute_time_t t = from_us_since_boot(us);
    sleep_until(t);
}
*/

semaphore_t sem;
void sleep_until_us(uint64_t us) {
    absolute_time_t t = from_us_since_boot(us);
    sem_acquire_block_until(&sem, t);
}



// Core 0 Main Code
int main(void){
    stdio_init_all();
    sem_init(&sem, 0, 10);
    uint64_t t = current_time_us();
    while (true) {
        t += 10000;
        sleep_until(t);
        uint64_t actual = current_time_us();
        printf("actual: %lld, expected: %lld\n", actual, t);
    }
}
