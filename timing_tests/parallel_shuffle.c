#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"



uint64_t current_time_us() {
    return to_us_since_boot(get_absolute_time());
}

// poorly shuffles an array of length SHUFFLE_LEN
#define SHUFFLE_LEN (256 + 16)
void __attribute__((noinline)) shuffle_bad(int* ar, int n) {
    int r = 123;
    for (int a = 0; a < n; a++) {
        int ind = r & 255;
        r = (r + a);
        r = (r << 3) | ((r >> 29) & 7);

        int* segment = &ar[ind];
        int first = segment[0];
        segment[0] = segment[1];
        segment[1] = segment[2];
        segment[2] = segment[3];
        segment[3] = segment[4];
        segment[4] = segment[5];
        segment[5] = segment[6];
        segment[6] = segment[7];
        segment[7] = segment[8];
        segment[8] = segment[9];
        segment[9] = segment[10];
        segment[10] = segment[11];
        segment[11] = segment[12];
        segment[12] = segment[13];
        segment[13] = segment[14];
        segment[14] = segment[15];
        segment[15] = segment[16];
        segment[16] = first;
    }
}


int useless(int x);



// Core 0 Main Code
int main(void){
    stdio_init_all();
    sleep_ms(1000);

    while (true) {
        uint64_t t = current_time_us();
        int r = useless(10000);
        t = current_time_us() - t;
        printf("Timing shuffle %d %d %lld\n", get_core_num(), r, t);
        sleep_ms(1000);
    }
}
