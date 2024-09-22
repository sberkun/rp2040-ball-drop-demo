#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#include "../actors.h"


uint64_t current_time_us() {
    return to_us_since_boot(get_absolute_time());
}

void matmul(int n, int* result_mat, int* a_mat, int* b_mat);

#define MAT_N 10

void __noinline bench() {
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
