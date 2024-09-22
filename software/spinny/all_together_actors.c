#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"

#include "actors.h"
#include "demo_pins.h"




#define SLOW_ST 3000
#define FAST_SLOWER_ST 70
#define FAST_ST 60
#define FAST_FASTER_ST 50


event_queue_t q;
#define MAIN_ACTOR 0
#define MOTOR_ACTOR 1
#define BUTTON_ACTOR 2
#define SENSORS_ACTOR 3
#define NUM_ACTORS 4 

// all actions
void motor_startup(uint64_t logical_time, size_t actor_id, void* arg);
void motor_set_countdown(uint64_t logical_time, size_t actor_id, void* arg);
void motor_set_tunnel_time(uint64_t logical_time, size_t actor_id, void* arg);
void motor_set_target_st(uint64_t logical_time, size_t actor_id, void* arg);
void motor_loop(uint64_t logical_time, size_t actor_id, void* arg);
void main_button_pressed(uint64_t logical_time, size_t actor_id, void* arg);
void main_loop(uint64_t logical_time, size_t actor_id, void* arg);
void button_check(uint64_t logical_time, size_t actor_id, void* arg);
void sensors_check(uint64_t logical_time, size_t actor_id, void* arg);

int main(void){
    stdio_init_all();
    event_queue_init(&q, 10, NUM_ACTORS);
    schedule_event(&q, to_us_since_boot(get_absolute_time()),
        MOTOR_ACTOR, motor_startup, NULL);
    work(&q);
}

#define SEND0(id, action) schedule_event(&q, logical_time, id, action, NULL)
#define SEND1(id, action, arg) schedule_event(&q, logical_time, id, action, (void *)(arg))

//////// STEPPER MOTOR ACTOR ////////

// this actor can be in two "modes":
// - if countdown is 0, it will speed up / slow down to match target_st
// - if countdown is nonzero, it automatically adjusts target_st based on
//   countdown and expected_tunnel_time

uint64_t actual_step_with_mul = SLOW_ST * ACCEL_MUL;
uint64_t target_st = SLOW_ST;
int countdown = 0;
uint64_t expected_tunnel_time = 0; 
int motor_toggle = 0;
int motor_position = 0;


void motor_set_countdown(uint64_t logical_time, size_t actor_id, void* arg) {
    countdown = (int) arg;
}

void motor_set_tunnel_time(uint64_t logical_time, size_t actor_id, void* arg) {
    expected_tunnel_time = logical_time + ((uint32_t) arg);
}

void motor_set_target_st(uint64_t logical_time, size_t actor_id, void* arg) {
    target_st = (int) arg;
}

void motor_startup(uint64_t logical_time, size_t actor_id, void* arg) {
    initialize_demo_pins();
    for (int a = 0; a < 16; a++) {
        gpio_put(STEP_PIN, 1);
        sleep_ms(2);
        gpio_put(STEP_PIN, 0);
        sleep_ms(2);
    }
    schedule_event(&q, logical_time + 4000000, MOTOR_ACTOR, motor_loop, NULL);
}

void motor_loop(uint64_t logical_time, size_t actor_id, void* arg) {
    gpio_put(STEP_PIN, motor_toggle);
    motor_toggle = !motor_toggle;
    motor_position += 1;
    if (motor_position == STEPS_PER_HALF_ROT) {
        motor_position = 0;
    }
    
    // adjust target_st based on countdown (if countdown is active)
    if (countdown > 0) {
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

    SEND1(MAIN_ACTOR, main_loop, motor_position);
    schedule_event(&q, logical_time + (actual_step_with_mul / ACCEL_MUL),
                   MOTOR_ACTOR, motor_loop, NULL);
}

/////////////////////////////////////


///////////// MAIN ACTOR ////////////

// state = 0 - pre-arm, motor slowly spinning. button press ->
// state = 1 - armed, motor spinning fast. button press ->
// state = 2 - waiting for correct position. when reached, unarm
// state = 3 - falling! waiting for countdown

int state = 0;
int target_pos_to_drop = (STEPS_PER_HALF_ROT - ((US_AFTER_E0 / FAST_ST) % STEPS_PER_HALF_ROT)) % STEPS_PER_HALF_ROT;
uint64_t reset_time = 0;


void main_button_pressed(uint64_t logical_time, size_t actor_id, void* arg) {
    if (state == 0) {
        state = 1;
        gpio_put(MAGNET_PIN, true);
        SEND1(MOTOR_ACTOR, motor_set_target_st, FAST_ST);
    } else if (state == 1) {
        state = 2;
        printf("drop position: %d %d\n", US_AFTER_E0, target_pos_to_drop);
    }
}

void main_loop(uint64_t logical_time, size_t actor_id, void* arg) {
    if (state < 2) {
        SEND0(BUTTON_ACTOR, button_check);
    } else if (state == 2) {
        int motor_position = (int) arg;
        if (motor_position == target_pos_to_drop) {
            state = 3;
            reset_time = logical_time + US_AFTER_E0 + 500000;
            gpio_put(MAGNET_PIN, false);
            SEND1(MOTOR_ACTOR, motor_set_tunnel_time, US_AFTER_E0);
            SEND1(MOTOR_ACTOR, motor_set_countdown, US_AFTER_E0 / FAST_ST);
        }
    } else { // state 3
        SEND0(SENSORS_ACTOR, sensors_check);
        if (logical_time > reset_time) {
            state = 0;
            printf("slowing down\n");
            SEND1(MOTOR_ACTOR, motor_set_target_st, SLOW_ST);
        }
    }
}

/////////////////////////////////////


//////////// BUTTON ACTOR ///////////

uint64_t arcade_button_last_pressed = 0;
bool arcade_button_saved = true;

void button_check(uint64_t logical_time, size_t actor_id, void* arg) {
    // check button - 1s debounce
    int arcade_button_new = gpio_get(ARCADE_BUTTON_PIN);
    if (!arcade_button_new && arcade_button_saved && logical_time - arcade_button_last_pressed > 1000000) {
        arcade_button_last_pressed = logical_time;
        printf("button!\n");
        SEND0(MAIN_ACTOR, main_button_pressed);
    }
    arcade_button_saved = arcade_button_new;
}

/////////////////////////////////////


/////////// SENSORS ACTOR ///////////

bool sense_1_saved = false;
bool sense_2_saved = false;

void sensors_check(uint64_t logical_time, size_t actor_id, void* arg) {
    uint64_t time = to_us_since_boot(get_absolute_time());
    uint64_t new_expected_tunnel_time = 0;
    
    bool sense_1_new = gpio_get(SENSE_1_PIN);
    bool sense_2_new = gpio_get(SENSE_2_PIN);
    if (sense_1_new && !sense_1_saved) { // event 1
        printf("lag: %lld\n", time - logical_time);
        new_expected_tunnel_time = time + US_AFTER_E1;
        printf("event 1: %lld\n", new_expected_tunnel_time - expected_tunnel_time);
    }
    if (!sense_1_new && sense_1_saved) { // event 2
        printf("lag: %lld\n", time - logical_time);
        new_expected_tunnel_time = time + US_AFTER_E2;
        printf("event 2: %lld\n", new_expected_tunnel_time - expected_tunnel_time);
    }
    if (sense_2_new && !sense_2_saved) { // event 3
        printf("lag: %lld\n", time - logical_time);
        new_expected_tunnel_time = time + US_AFTER_E3;
        printf("event 3: %lld\n", new_expected_tunnel_time - expected_tunnel_time);
    }
    if (!sense_2_new && sense_2_saved) { // event 4
        printf("lag: %lld\n", time - logical_time);
        new_expected_tunnel_time = time + US_AFTER_E4;
        printf("event 4: %lld\n", new_expected_tunnel_time - expected_tunnel_time);
    }
    sense_1_saved = sense_1_new;
    sense_2_saved = sense_2_new;

    if (new_expected_tunnel_time != 0) {
        int delta = new_expected_tunnel_time - logical_time;
        SEND1(MOTOR_ACTOR, motor_set_tunnel_time, delta);
    }
}

/////////////////////////////////////
