


#define MAGNET_PIN PICO_DEFAULT_LED_PIN
#define SENSE_1_PIN 18
#define SENSE_2_PIN 19
#define STEP_PIN 20

#define STEPS_PER_HALF_ROT 1600


// Event timings:
// - E0: ball is released from magnet
// - E1: top of sense1
// - E2: bottom of sense1
// - E3: top of sense2
// - E4: bottom of sense2
// - E5: ball passes through disk
// the first 4 timings can be measured: the last one is guesstimated based on E3-E4 timing

#define US_AFTER_E4 4500
#define US_AFTER_E3 US_AFTER_E4 + 3100
#define US_AFTER_E2 US_AFTER_E3 + 19200
#define US_AFTER_E1 US_AFTER_E2 + 3100
#define US_AFTER_E0 US_AFTER_E1 + 157000

