#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "math.h"


uint64_t current_time_us() {
    return to_us_since_boot(get_absolute_time());
}

void bench_fln() {
    int trials = 5000;
    uint64_t t = current_time_us();
    float cheese = 3.1415926e+30;
    float delta = 1e+25;
    float total = 0;
    for (int a = 0; a < trials; a++) {
        total += log(cheese);
        cheese += delta;
    }
    t = current_time_us() - t;
    printf("Timing fln %d %f %lld\n", get_core_num(), total, t);
}


// Core 0 Main Code
int main(void){
    stdio_init_all();
    sleep_ms(1000);

    int r = 0;
    while (true) {
        bench_fln();
        sleep_ms(1000);
    }
}
