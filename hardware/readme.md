# Hardware

## Ports, Usages, Pins, etc.

| Port    | Usage      | Pin | Notes                    |
|---------|------------|-----|--------------------------|
| NRST    | GND        |  1  |                          |
| PA1     | SenseLED1+ |  2  | *A1                      |
| PA2     | GLED3      |  3  | *A0                      |
| VSS     | GND        |  4  |                          |
| PD0     | GLED2      |  5  | *Non-A                   |
| VDD     | VCC        |  6  |                          |
| PC0     | GLED1      |  7  | *Non-A                   |
| SDA     | SDA        |  8  | PC1                      |
| SCL     | SCL        |  9  | PC2                      |
| PC3     | Button1    | 10  | to GND, Pull-up          |
| PC4     | Button2    | 11  | to GND, Pull-up          |
| PC5     | SAO GPIO2  | 12  |                          |
| MOSI    | Neopixel   | 13  | PC6                      |
| PC7     | SAO GPIO1  | 14  |                          |
| SWIO    | SWIO       | 15  | PD1, Programer, exp. pad |
| PD2     | SenseLED2- | 16  | *A3                      |
| PD3     | SenseLED2+ | 17  | *A4                      |
| PD4     | SenseLED1- | 18  | *A7                      |
| UTX     | UTX        | 19  | PD5, USART, exposed pad  |
| URX     | URX        | 20  | PD6, USART, exposed pad  |
| VSS     | GND        | 21  |                          |


## Sense LEDs

Two LEDs are both sunk and sourced by the MCU so it can run both in forward and reverse bias (sensing) modes. 


## CH32 Product line part number notes
In case you were curious, Ex: CH32V003F4U6

- CH32 = product line.
- V = QingKe RISC-V-based
- 0 = QingKe V2 core (no FP)
- 03 = General Purpose
- F = 20 pins
- 4 = 16 Kbytes of Flash memory 
- P/U = TSSOP/QFN respectively
- 6 = -40℃～85℃ (industrial-grade) 
