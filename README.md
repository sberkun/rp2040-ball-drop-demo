# Samuel Berkun's master's thesis work

This repo contains some stuff I worked on during my master's thesis. Namely:
 - a C library for timing-aware actors
 - several timing tests
 - a "tunneling ball" demonstration


## LTA (Library for Timing-aware Actors)

LTA consists of [actors.c](./software/lib/actors.c) and [actors.h](./software/lib/actors.h), and is written for the RP2040. Its main structure is the event queue, which is similar to the event queue in the internals of Lingua Franca. It can be used as a very lightweight way to build concurrent programs; if more features are needed, those programs can be migrated to Lingua Franca.

## Timing Tests

My thesis contains several performance tests comparing bare metal, LTA, LF, and FreeRTOS on the RP2040. The bare metal and LTA code can be found [here](./software/timing_baremetal/) and [here](./software/timing_actors/) respectively.


## Ball Drop Demonstration

The hardware (STLs, bill of materials, wiring diagram) can be found in the [hardware folder](./hardware/). There are two different versions of the software:
 - one version was written using Lingua Franca, which can be found [here](./software/lf_src/).
 - one version was written using LTA, which can be found [here](./software/spinny/). 
