#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"



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


uint64_t glob_start;

void core1_entry() {
    uint64_t tt = glob_start;
    while (true) {
        tt += 1000000;
        sleep_until(from_us_since_boot(tt));
        bench();
    }
}

// Core 0 Main Code
int main(void){
    stdio_init_all();
    sleep_ms(1000);

    glob_start = current_time_us();

    multicore_launch_core1(core1_entry);

    uint64_t tt = glob_start;
    while (true) {
        tt += 1000000;
        sleep_until(from_us_since_boot(tt));
        bench();
    }
}
