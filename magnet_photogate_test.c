#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"

#include "action_queue.h"

#include "demo_pins.h"


// Interrupt handler for the button press
void gpio_isr(uint gpio, uint32_t event_mask) {
    printf("event on %d %x\n", gpio, event_mask);
}



// Core 0 Main Code
int main(void){
    stdio_init_all();

    // Initialize the magnet pin as an output
    gpio_init(MAGNET_PIN);
    gpio_set_dir(MAGNET_PIN, GPIO_OUT);

    // Initialize the stepper driver pin as an output
    gpio_init(MAGNET_PIN);
    gpio_set_dir(MAGNET_PIN, GPIO_OUT);

    // Initialize sense pins as pull-up inputs
    gpio_init(SENSE_1_PIN);
    gpio_set_dir(SENSE_1_PIN, GPIO_IN);
    gpio_pull_up(SENSE_1_PIN);
    gpio_init(SENSE_2_PIN);
    gpio_set_dir(SENSE_2_PIN, GPIO_IN);
    gpio_pull_up(SENSE_2_PIN);

    // Enable edge=triggered interrupts for sense pins
    gpio_set_irq_enabled(SENSE_1_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(SENSE_2_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_callback(gpio_isr);
    irq_set_enabled(IO_IRQ_BANK0, true);
    int l = 0;
    while (true) {
        // Add a small delay to debounce the button
        // int sense_1_state = gpio_get(SENSE_1_PIN);
        // if (sense_1_state != sense_1_stored_state) {
        //     sense_1_stored_state = sense_1_state;
        //     printf("sense 1 change: %d\n", sense_1_state);
        // }
        // if (l++ % 10000 == 0) {
        //     printf(".\n");
        // }
        sleep_us(50);
    }
}
