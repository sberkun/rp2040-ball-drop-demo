#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"



uint64_t current_time_us() {
    return to_us_since_boot(get_absolute_time());
}

// multiplies a and b, storing the result in result
// all are assumed to be n by n
// void __attribute__((noinline)) matmul(int n, int* result_mat, int* a_mat, int* b_mat) {
//     int r = 0;
//     for (int i = 0; i < n; i++) {
//         for (int j = 0; j < n; j++) {
//             result_mat[r] = 0;
//             for (int k = 0; k < n; k++) {
//                 result_mat[r] += a_mat[i*n + k] * b_mat[k*n + j];
//             }
//             r++;
//         }
//     }
// }

void matmul(int n, int* result_mat, int* a_mat, int* b_mat);

#define MAT_N 10

void bench() {
    int trials = 400;
    int a_mat[MAT_N * MAT_N];
    int b_mat[MAT_N * MAT_N];
    int c_mat[MAT_N * MAT_N];
    for (int i = 0; i < MAT_N * MAT_N; i++) {
        a_mat[i] = i;
        b_mat[i] = i + 3;
    }
    uint64_t t = current_time_us();
    for (int i = 0; i < trials; i++) {
        matmul(MAT_N, c_mat, a_mat, b_mat);
    }
    t = current_time_us() - t;
    printf("Timing matmul %d %d %lld\n", get_core_num(), c_mat[0], t);
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
