# Samuel Berkun's master's thesis work

This repo contains some stuff I worked on during my master's thesis. Namely:
 - a C library for timing-aware actors
 - several timing tests
 - code for a "tunneling ball" demonstration

# Ball Drop Demonstration

This is code to drop a ball through a spinning disk.


## Action Queue

The "action queue" is similar to the event queue in the internals of Lingua Franca. It can be used as a very lightweight way to build concurrent programs; if more features are needed, those programs can be migrated to Lingua Franca.

