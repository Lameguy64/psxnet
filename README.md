# PSXNET
A simple library and client program that will allow the original PlayStation to connect to the internet via a serial link connection to a Raspberry Pi. Made mostly for fun and as a silly little proof of concept showing that a console from the 90s can connect to the internet.

It supports basic TCP and UDP (untested) communications as well as connecting via DNS resolve. There's no support for https out of the box but improvements to both the library and client programs are encouraged.

Compiling the example program will require the PsyQ PlayStation SDK and msys for make and related GNU utilities. The included client program must be compiled and run on a Raspberry Pi.