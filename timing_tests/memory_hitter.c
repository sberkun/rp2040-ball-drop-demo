#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"



uint64_t current_time_us() {
    return to_us_since_boot(get_absolute_time());
}


#define SIZE 400
int memory_hitter(int n, int r) {
    int ar[SIZE];
    for (int a = 0; a < SIZE; a++) {
        ar[a] = 0;
    }
    for (int a = 0; a < n; a++) {
        for (int b = (a % 7); b + 7 < SIZE; b+=7) {
            ar[b + 0] += a;
            ar[b + 1] += a;
            ar[b + 2] += a;
            ar[b + 3] += a;
            ar[b + 4] += a;
            ar[b + 5] += a;
            ar[b + 6] += a;
        }
    }
    int total = 0;
    for (int a = 0; a < SIZE; a++) {
        total += ar[a];
    }
    
    return total + r;
}


// Core 0 Main Code
int main(void){
    stdio_init_all();
    sleep_ms(1000);

    int r = 0;
    while (true) {
        uint64_t t = current_time_us();
        r = memory_hitter(1000, r);
        t = current_time_us() - t;
        printf("Timing memory hitter A %d %d %lld\n", get_core_num(), r, t);
        sleep_ms(1000);
    }
}
