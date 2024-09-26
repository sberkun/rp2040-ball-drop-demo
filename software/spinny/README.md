# Tunneling Ball Demo Software

This is software written for the Raspberry Pi Pico. If you have a different microcontroller this will not work.

TODO

## Quickstart

### Part 1: Testing the button, electromagnet, and photogates 

 - If you haven't, download the [pico sdk](https://github.com/raspberrypi/pico-sdk/)
 - Set the `PICO_SDK_PATH` environment variable to the location of the pico sdk.
 - Change the pin numbers in [demo_pins.h](./demo_pins.h) to match the actual pin numbers things are plugged into.
 - Build the project. Suggested commands:
 ```
 mkdir build
 cd build
 cmake .. && make
 ```
 - At this point you should have `spinny.uf2` in your `build` folder. Plug in your raspberry pi pico and copy `spinny.uf2` into it to flash it.
 - Open up a serial terminal. For example, here is a command that often works on Linux: 
 ```
 minicom -w -b 115200 -o -D /dev/ttyACM0
 ```

TODO

### Part 2: Testing the motor

TODO

### Part 3: Full Demo

TODO


TODO fix all the cmakelists to not dox myself
TODO fix all the imports of actors.h