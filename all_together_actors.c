#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"

#include "demo_pins.h"




#define SLOW_ST 3000
#define FAST_SLOWER_ST 70
#define FAST_ST 60
#define FAST_FASTER_ST 50


event_queue_t q;
#define MAIN_ACTOR 0
#define MOTOR_ACTOR 1
#define SENSORS_ACTOR 2
#define MAGNET_ACTOR 3
#define BUTTON_ACTOR 4



//////// STEPPER MOTOR ACTOR ////////

// this actor can be in two "modes":
// - if countdown is 0, it will speed up / slow down to match target_st
// - if countdown is nonzero, it automatically adjusts target_st based on
//   countdown and expected_tunnel_time

uint64_t actual_step_with_mul;
uint64_t target_st;
int countdown;
uint64_t expected_tunnel_time; 
int motor_saved;
int motor_position;


void motor_init() {
    actual_step_with_mul = SLOW_ST * ACCEL_MUL;
    target_st = SLOW_ST;
    countdown = 0;
    expected_tunnel_time = 0;
    motor_saved = 0;
    motor_position = 0;
}

void motor_set_countdown(uint64_t logical_time, size_t actor_id, void* arg) {
    countdown = (int) arg;
}

void motor_set_tunnel_time(uint64_t logical_time, size_t actor_id, void* arg) {
    expected_tunnel_time = logical_time + ((uint32_t) arg);
}

void motor_loop(uint64_t logical_time, size_t actor_id, void* arg) {
    gpio_put(STEP_PIN, motor_saved);
    motor_saved = !motor_saved;
    motor_position += 1;
    if (motor_position == STEPS_PER_HALF_ROT) {
        motor_position = 0;
    }
    
    // adjust target_st based on countdown (if countdown is active)
    if (countdown != 0) {
        uint64_t actual_st = actual_step_with_mul / ACCEL_MUL;
        uint64_t disk_tunnel_time = logical_time + (actual_st * countdown);
        if (disk_tunnel_time < expected_tunnel_time) {
            // disk will arrive first: slow down!
            target_st = FAST_SLOWER_ST;
        } else {
            // ball will arrive first: speed up!
            target_st = FAST_FASTER_ST;
        }
        countdown -= 1;
    }

    // adjust actual_step_with_mul based on target_st
    uint64_t actual_st = actual_step_with_mul / ACCEL_MUL;
    if (actual_st > target_st) {
        actual_step_with_mul -= actual_st;
    }
    if (actual_st < target_st) {
        actual_step_with_mul += actual_st;
    }

    schedule_event(&q, logical_time, MAIN_ACTOR, main_loop, motor_position);
    schedule_event(&q, logical_time + (actual_step_with_mul / ACCEL_MUL),
                   MOTOR_ACTOR, motor_loop, NULL);
}

/////////////////////////////////////




//////// MAIN ACTOR ////////


void main_init() {
    
}

void main_loop(uint64_t logical_time, size_t actor_id, void* arg) {
    int motor_position


    schedule_event(&q, logical_time + (actual_step_with_mul / ACCEL_MUL),
                   MOTOR_ACTOR, motor_loop, NULL);
}