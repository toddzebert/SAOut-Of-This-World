# Firmware for SAOut Of This World

## Setup
1) Install VSCode
2) Install the PlatformIO extension in VSCode
3) Clone the [ch32v003fun](https://github.com/cnlohr/ch32v003fun) repo
4) Follow the [installation](https://github.com/cnlohr/ch32v003fun/wiki/Installation) guide
5) Open it with PlatformIO and let it set up
6) Open the SAOut Of This World firmware project with PlatformIO
7) Let it set up, then click Build to see if it worked

# Description

At its core, its a synchronous finite state machine; it's not (directly) event-driven, because events are "buffered" into the synchronous tick/tock of the main loop. It more or less follows the [UML state machine](https://en.wikipedia.org/wiki/UML_state_machine). It's written in straight C.

It uses the [ch32v003fun](https://github.com/cnlohr/ch32v003fun) "environment" to avoid the manufacturer's weighty HAL/EVT; the community around it has been very helpful. We also use PlatformIO to build the project.

Conceptually, initially an init event is passed to each Thing. Then there's the main loop, which communicates with Things, which in turn communicates with Effects. Effects each have a timer, based on tocks, associated with a Thing. When the timer expires, a run event is passed to each Thing which in turn passes on the event to the Effect. Other input events are passed similarly via the main loop. Another way of thinking about it is kinda like PubSub, but Things don't have to "subscribe", we "publish" all events to all Things (then passed on to Effects).

Other input events include i2c writes, buttons and the "sense" LEDs.

While both Things and Effects maintain there own UML "action" state (ENTER, GO, EXIT, etc), so far there's no need for Things to do so. Effects also maintain their own internal state as needed.

The MCU is the CH32V003F4U6 (QFN-20) chip, with 2 buttons, 16 WS2812s (in one channel) and five LEDs, two of which are wired special - see the hardware readme.

## Tick/Tock
`SysClick` is used to create a synchronous 0.1ms "tick", and every 10 there's a "tock" (1ms). The main loop runs on the tocks. The tick is meant to provide PWM of the LEDs.


## TODO

* Fix "PA" GPIO pins - in progress.
* Fix Comet effect
* Finish & test i2c
* Finish & test buttons
* Make LEDs PWM - see https://discord.com/channels/@me/1170888366540197899/1301040627512770580 
* Add more effects

## Future ideas

* Right now Things and Effects return `dirty` to alert the main loop that the WS' need to be updated, and timing intervals (delays) are set in a global var. I'd like to uncouple that so they return a struct with both dirty and delay, and maybe more. I want to avoid putting too much on the stack though, although I suspect the call stack is never more than a few calls deep: Main Loop, Thing, Effect handler, Effect State handler, and perhaps a helper routine. There's no recursion.
* Twinkle effect: add normally on and normally off modes.
* Add the "CC3K" effect.

## CH32V003FUN Platform Notes

* To get a random binary: `rnd_fun(0, 1) & 0x01)`
* To get a 0-255 randomly: `(uint8_t)rnd_fun(0, 1)`

## Thanks.
Special thanks to cnlohr for the [ch32v003fun](https://github.com/cnlohr/ch32v003fun) project,
and all the helpful people on his Discord server.
