#include "hardware/gpio.h"


#define MAGNET_PIN 22
#define SENSE_1_PIN 27
#define SENSE_2_PIN 26
#define STEP_PIN 16
#define ARCADE_BUTTON_PIN 17

#define STEPS_PER_HALF_ROT 1600
#define ACCEL_MUL 1024


// Event timings:
// - E0: ball is released from magnet
// - E1: top of sense1
// - E2: bottom of sense1
// - E3: top of sense2
// - E4: bottom of sense2
// - E5: ball passes through disk
// the first 4 timings can be measured: the last one is guesstimated based on E3-E4 timing

#define US_AFTER_E4 (4500)
#define US_AFTER_E3 (US_AFTER_E4 + 3100)
#define US_AFTER_E2 (US_AFTER_E3 + 19500)
#define US_AFTER_E1 (US_AFTER_E2 + 3500)
#define US_AFTER_E0 (US_AFTER_E1 + 263000)



static inline void initialize_demo_pins() {
    // Initialize the magnet pin as an output
    gpio_init(MAGNET_PIN);
    gpio_set_dir(MAGNET_PIN, GPIO_OUT);

    // Initialize the stepper driver pin as an output
    gpio_init(STEP_PIN);
    gpio_set_dir(STEP_PIN, GPIO_OUT);

    // Initialize sense pins as pull-up inputs
    gpio_init(SENSE_1_PIN);
    gpio_set_dir(SENSE_1_PIN, GPIO_IN);
    gpio_pull_up(SENSE_1_PIN);
    gpio_init(SENSE_2_PIN);
    gpio_set_dir(SENSE_2_PIN, GPIO_IN);
    gpio_pull_up(SENSE_2_PIN);

    // Initialize button pin as pull-up input
    gpio_init(ARCADE_BUTTON_PIN);
    gpio_set_dir(ARCADE_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(ARCADE_BUTTON_PIN);
}