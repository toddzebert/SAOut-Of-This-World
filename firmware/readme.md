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

At its core, its a synchronous finite state machine; it's not (directly) event-driven, because events are "buffered" into the synchronous tock of the main loop. It more or less follows the [UML state machine](https://en.wikipedia.org/wiki/UML_state_machine). It's written in straight C.

It uses the [ch32v003fun](https://github.com/cnlohr/ch32v003fun) "environment" to avoid the manufacturer's weighty HAL/EVT; the community around it has been very helpful. We also use PlatformIO to build the project.

Conceptually, initially an init Event is passed to each Thing. Then there's the main loop, which communicates with Things. There are two types of Things: input and output. Output Things (like Stars,Eyes, Trim) are associated with and communicate with Effects (like Twinkle, Blink, Rotate). Input Things (like Buttons, GPIO) do not have associated Effects.

Effects each have a timer, based on tocks, associated with a Thing. When the timer expires, a Run Event is passed to each Thing which in turn passes on the event to the Effect. Other input events are passed similarly via the main loop.

Another way of thinking about it is kinda like PubSub, but Things don't have to "subscribe", we "publish" all events to all Things (then passed on to Effects). Currently, Effects return a "dirty" flag, which is then returned by the corresponding Thing, to alert the main loop that the WS2812s need to be updated.

Other input events include i2c writes, (SAO) GPIO inputs, buttons and the "sense" LEDs. Most are "polled" with the exception of the i2c writes (handled by an ISR). Polled Things return an Event, to be processed in the main loop. The i2c is unique and isn't really a Thing; it "sends" Events by setting a "Global Event" var which again, is processed in the main loop. 

While both Things and Effects maintain there own UML "action" state (ENTER, GO, EXIT, etc), as do "output" Things, but so far there's no need for "output" Things to do so. Things and Effects also maintain their own internal state, as needed.

The next most important concept is the "registry". Because this SAO is meant to be an i2c target, all the settings and outputs are stored in the registry. The i2c library modifies the registry directly when it receives a write. Things and Effects depend on both reading and writing to the registry. Any i2c controller can switch Effect "modes", settings, or even control the LEDs and WS2812s directly using the "Raw" Effect.

For Stars, the 8bit value is passed through a gamma map table, then its reduced to 6 bits for the LED soft PWM.

The MCU is the CH32V003F4U6 (QFN-20) chip, with 2 buttons, 16 WS2812s (in one channel) and five (soft PWM)LEDs, two of which are wired special - see the hardware readme.

When tested during supercon 2024, it was pulling high 30's to mid 40's ma.

## Tock

`SysClick` is used to create a synchronous 1ms "tock". The main loop runs on the tocks.

## i2c Use
@todo
* I2C_ADDRESS 0x09

## Libraries (/lib)
* Color Utilities
* i2c_slave
  * Modified from example - thanks [true] - to:
    * better handle errors, especially stop.
    * Not call callbacks when i2c IDs don't match.
    * Increase priority.
* rand
* ws2812b_dma_spi_led_driver

## TODO & Progress

* [In progress] Finish & test i2c. Probably need a "dirty" reg entry - nope, write callback can determine.
* [Emit done, queue added, "listeners" unstarted] Things' (input) events.
* Enhance and add Effects
* [Deprioritized] Fix Comet effect
* [Hold] Sense LEDs

## Change Log

* [Done] Buttons
* [Done] Events base code
* [Done, dev error] Fix "PA" GPIO pins - thanks Discord
* [Done] reduce GPIO speeds
* [Done] fix i2c crashing - thanks[true]
* [Done] Make LEDs (soft) PWM - thanks [true]
  
## Future ideas

* The mismatch of the length of `enum Things_t` and arrays such as `reg_thing_start[]` could be an issue in the future.
* Right now Things and Effects return `dirty` to alert the main loop that the WS' need to be updated, and timing intervals (delays) are set in a global var. I'd like to uncouple that so they return a struct with both dirty and delay, and maybe more. I want to avoid putting too much on the stack though, although I suspect the call stack is never more than a few calls deep: Main Loop, Thing, Effect handler, Effect State handler, and perhaps a helper routine. There's no recursion.
* Twinkle effect: add normally on and normally off modes.
* Add the "CC3K" effect.
* Move i2c stuff from main to its own file, and DRY out `main()` including the main loop.

## CH32V003FUN Platform Notes

* To get a random binary: `rnd_fun(0, 1) & 0x01)`
* To get a 0-255 randomly: `(uint8_t)rnd_fun(0, 1)`
* It does not support `malloc`.
* For best RISC performance, focus on 0's in loops/counters.
* The i2c_slave/src/i2c_slave.h is modified from the fun example's version to clear stop condition on error, and increase IRQ priority.
* Do not use `Delay_Ms()` as it is incompatible with 

## Thanks.
Special thanks to cnlohr for the [ch32v003fun](https://github.com/cnlohr/ch32v003fun) project,
and all the helpful people on his Discord server. And to [true] at the Hackaday Supercon 2024.
