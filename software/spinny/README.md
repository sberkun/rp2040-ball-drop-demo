# Tunneling Ball Demo Software

This is software written for the Raspberry Pi Pico. If you have a different microcontroller this will not work.

## Quickstart

### Part 1: Testing the button, electromagnet, and photogates 

To flash the test program for the first time:
 - If you haven't, download the [pico sdk](https://github.com/raspberrypi/pico-sdk/)
 - Set the `PICO_SDK_PATH` environment variable to the location of the pico sdk.
 - Change the pin numbers in [demo_pins.h](./demo_pins.h) to match the actual pin numbers things are plugged into.
 - Build the project. Suggested commands (run in the `spinny/` folder):
 ```
 mkdir build
 cd build
 cmake .. && make
 ```
 - At this point you should have `spinny.uf2` in your `build` folder. Plug in your raspberry pi pico (holding down the bootsel button) and copy `spinny.uf2` into it to flash it.
 - Open up a serial terminal. For example, here is a command that often works on Linux: 
 ```
 minicom -w -b 115200 -o -D /dev/ttyACM0
 ```
 - Align the disk such that one of the holes is beneath the tube.
 - Plug in the device's 12v power supply so that the electromagnet is powered.

If you reach this point, congratulations! The test program is running and you are ready to calibrate the device.
 - Push the button. In the serial terminal, a bunch of lines saying `ab: <numbers>` should appear. If not, the button isn't working.
 - Hold down the button, and hold a bearing ball near the electromagnet. The ball should stick when the button is held down. If not, the electromagnet isn't working.
 - Release the button. The electromagnet should release the bearing ball, and the serial terminal should print something like:
 ```
 sense1: 1 265902                                                                
 sense1: 0 3662                                                                  
 sense2: 1 17052                                                                 
 sense2: 0 3349  
 ```
 - In the above lines, `sense1` and `sense2` correspond to the top and bottom photogates respectively. The `1` and `0` represent entering and leaving the photogate, and the last number represents the elapsed time since the last event. If you see these lines, the sensors are working.
 - Perform a few more drops (hold button, place ball, release button, record times), then record the median times in `demo_pins.h`. For example, the above numbers correspond to:
 ```
 #define US_AFTER_E3 (US_AFTER_E4 + 3349)
 #define US_AFTER_E2 (US_AFTER_E3 + 17052)
 #define US_AFTER_E1 (US_AFTER_E2 + 3662)
 #define US_AFTER_E0 (US_AFTER_E1 + 265902)
 ``` 


### Part 2: Testing the motor

In `CMakeLists.txt`, comment out `magnet_photogate_test.c`, and uncomment `motor_spin_test.c`. Then, flash the device as before, and plug in the 12v power supply.

The disk should spin slowly. Press the button, and it should spin quickly. Press the button again, and it should spin slowly again.

If the disk locks up instead of spinning quickly, unplug it ASAP. You may have to change the top speed in `motor_spin_test.c`, by increasing the step time. Currently the "fast" step time is 40 microseconds, but this may have to be changed to 50 or above if the motor is too weak. If this is the case, you will probably have to lower the speeds for the full demo.

### Part 3: Full Demo

In `CMakeLists.txt`, comment out `motor_spin_test.c`, and uncomment `all_together_actors.c`. Unplug the 12v power supply, flash the device, but then unplug the device once it has been flashed.

Carefully align the disk. There is a pencil line on the disk to help with alignment; the pencil line should go directly beneath the front edge of the frame.

Plug in the 12v power supply, then plug in the device. The disk will spin slowly. Press the button and the disk will spin up, and you can place the ball bearing on the electromagnet. Press the button again to drop the ball.

If the ball goes through a hole in the disk, congratulations! Everything works. Otherwise, you have some work to do.
 - a good place to start is by lowering the speed. You can do this by increasing the step times at the top of `all_together_actors.c`.
 - open a serial monitor; the demo should print timing errors for each event. A few thousand microseconds is ok for the first event, and a few hundred microseconds is ok for the others. If the timing errors are large and consistent, you may have to recalibrate using `magnet_photogate_test.c`.
 - If the timing errors are small but the ball still bounces off the disk, `US_AFTER_E4` may be off, or the initial position of the disk may be off.
 - If you're using different software or hardware (i.e. Lingua Franca instead of the actor library, or an arduino nano instead of a raspberry pi pico), you may want to check that the main loop is running fast enough to keep up. One way to do so is with a logic analyzer on the step pin; make sure that the step pin is being driven at the intended frequency.
