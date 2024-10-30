# Firmware for SAOut Of This World

## Setup
1) Install VSCode
2) Install the PlatformIO extension in VSCode
3) Clone the [ch32v003fun](https://github.com/cnlohr/ch32v003fun) repo
4) Follow the [installation](https://github.com/cnlohr/ch32v003fun/wiki/Installation) guide
5) Open it with PlatformIO and let it set up
6) Open the SAOut Of This World firmware project with PlatformIO
7) Let it set up, then click Build to see if it worked


## Ports, Usages, Pins, etc.

| Port    | Usage      | Pin | Notes                    |
|---------|------------|-----|--------------------------|
| NRST    | GND        |  1  |                          |
| PA1     | SenseLED1+ |  2  |                          |
| PA2     | GLED3      |  3  |                          |
| VSS     | GND        |  4  |                          |
| PD0     | GLED2      |  5  |                          |
| VDD     | VCC        |  6  |                          |
| PC0     | GLED1      |  7  |                          |
| SDA     | SDA        |  8  | PC1                      |
| SCL     | SCL        |  9  | PC2                      |
| PC3     | Button1    | 10  |                          |
| PC4     | Button2    | 11  |                          |
|         | N/C        | 12  |                          |
| MOSI    | Neopixel   | 13  | PC6                      |
|         | N/C        | 14  | PC7                      |
| SWIO    | SWIO       | 15  | PD1, Programer, exp. pad |
| PD2     | SenseLED2- | 16  |                          |
| PD3     | SenseLED2+ | 17  |                          |
| PD4     | SenseLED1- | 18  |                          |
| UTX     | UTX        | 19  | PD5, USART, exposed pad  |
| URX     | URX        | 20  | PD6, USART, exposed pad  |
| VSS     | GND        | 21  |                          |

## CH32V003FUN Platform Notes

* To get a random binary: `rnd_fun(0, 1) & 0x01)`
* To get a 0-255 randomly: `(uint8_t)rnd_fun(0, 1)`

## Thanks.
Special thanks to cnlohr for the [ch32v003fun](https://github.com/cnlohr/ch32v003fun) project,
and all the helpful people on his Discord server.
