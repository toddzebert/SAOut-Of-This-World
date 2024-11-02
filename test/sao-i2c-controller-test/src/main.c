#include "ch32v003fun.h"

#include <stdio.h>
#include <stdint.h>


// #include <ch32v_hal.inl>
// Below not needed as the .inl file includes it.
#include "../lib/color_lcd/src/ch32v_hal.inl"


// I2C.
#define I2C_ADDRESS 0x09


uint8_t buf_in[128] = {0};
uint8_t buf_out[128] = {0};

int main()
{
    uint8_t reg = 0;

    SystemInit();

    Delay_Ms(500);

    printf("Hello World!\n");

    Delay_Ms(100);

    /*
    | SDA     | SDA        |  8  | PC1                      |
    | SCL     | SCL        |  9  | PC2                      |
    */
    I2CInit(PC1, PC2, 400000);

    Delay_Ms(100);

    if (I2CTest(I2C_ADDRESS) == 0)
    {
        printf("I2C test failed\n");
        return 1;
    }

    printf("I2C test passed\n");

    uint eye_mode_reg = 57; // Eyes mode
    buf_out[0] = eye_mode_reg;
    buf_out[1] = 0x01; // Random.

    uint eye_left_green_reg = 52; // Left eye green.

    while(1)
    {
        buf_out[0] = eye_mode_reg;
        // Mode 0x01 = Random
        // Mode 0x02 = Alternate
        buf_out[1] = buf_out[1] == 0x01 ? 0x02 : 0x01; // alternate

        printf("Mode %d\n", buf_out[1]);

        I2CWrite(reg, buf_out, 2);

        Delay_Ms(200);

        I2CRead(0x78, buf_in, 1);

        printf("Mode read %02x\n", buf_in[0]); // @debug

        // if (buf_in[0] == 0x01) printf("Eyes Mode 1\n");
        // else printf("Eyes Mode UNEXPECTED\n");

        Delay_Ms(5000);

        buf_out[eye_left_green_reg] = 0xff;

        I2CWrite(reg, buf_out, 1);

        Delay_Ms(5000);

        buf_out[eye_left_green_reg] = 0x00;

        I2CWrite(reg, buf_out, 1);

        Delay_Ms(5000);
    }





    return 0; 
}
