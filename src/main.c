/*
 * main.c
 *
*/

/*
    From ws2812b_dma_spi_led_driver.h:
    **For the CH32V003 this means output will be on PORTC Pin 6**
*/
#include "main.h"
#include <ch32v003fun.h>
#include <i2c_slave.h>
#include <stdio.h>

// > Note first n LEDs of DMA Buffer are 0's as a "break" (from lib).
#define LEDS 16 // @debug should be 16 but tried testing 18...
#define MIN_LED 0 // @debug testing
// This is default in .h but making it explicit. > Has to be divisible by 4.
#define DMALEDS 20 // For lib, for testing.

#define WS2812DMA_IMPLEMENTATION
#define WSRBG

#include <ws2812b_dma_spi_led_driver.h>

volatile uint8_t timer_tick = 0;

// @debug testing.
struct WsEffect comet = {
    "comet",
    "Comet",
    250,
    250,
    250,
    (uint32_t) 0xC00000,
    1,
    1,
    1,
    1,   
};

/*
struct WsSections ws_section_eyes = {
    "eyes",
    WS_EYES_LED_START,
    WS_EYES_LED_END,
    WS_EYES_LED_COUNT,
};
*/

/*
struct WsSections ws_section_upper_trim = {
    "upper_trim",
    WS_UPPER_TRIM_START,
    WS_UPPER_TRIM_END,
    WS_UPPER_TRIM_COUNT
};
*/

/*
struct WsSections ws_section_lower_trim = {
    "lower_trim",
    WS_LOWER_TRIM_START,
    WS_LOWER_TRIM_END,
    WS_LOWER_TRIM_COUNT,
};
*/

/* @debug not working, but not part of this branch's goal.
struct WsSections ws_section_lower_trim;
ws_section_lower_trim.machine_name = 'lower_trim';
*/


#define COMET_DEFAULT_TIMER_BASE 250 // 1/4s
uint16_t comet_timer_base = COMET_DEFAULT_TIMER_BASE;
#define COMET_LENGTH 3
/* red comet
const static uint32_t comet_colors[3] = {
    0xC00000, 0x300000, 0x100000,
};
*/
// /* blue comet
const static uint32_t comet_colors[3] = {
    0x0000c0, 0x000030, 0x000010,
};
// */

static int8_t comet_position = MIN_LED;
static int8_t comet_direction = 1; // or -1
volatile uint16_t comet_timer = COMET_DEFAULT_TIMER_BASE;
static volatile uint8_t comet_dirty = 1; // or 0


// Adapated from https://github.com/cnlohr/ch32v003fun/blob/master/examples/adc_fixed_fs/adc_fixed_fs.c .
// TIM1C1 uses PD2.
void init_timer() {
    // @todo change to use TIM2? Would also change ISR name.
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1; // > APB2 peripheral clock enable register.
    // @todo why both TIM1 and TIM2?
    TIM1->CTLR1 |= TIM_CounterMode_Up | TIM_CKD_DIV1;
    TIM1->CTLR2 = TIM_MMS_1;
    
    // @debug now at 1/4s
    TIM1->ATRLR = SOTW_ATRLR-1; // > Auto-reload value register; the counter stops when ATRLR is empty.
    TIM1->PSC = SOTW_PSC-1; // > Counting clock prescaler.
    TIM1->RPTCR = 0; // > Recurring count value register.
    TIM1->SWEVGR = TIM_PSCReloadMode_Immediate; // > Event generation register.

    NVIC_EnableIRQ(TIM1_UP_IRQn);
    TIM1->INTFR = ~TIM_FLAG_Update; // > Interrupt status register. 
    TIM1->DMAINTENR |= TIM_IT_Update; // > DMA/interrupt enable register.
    TIM1->CTLR1 |= TIM_CEN;
}

// Both adapted from https://github.com/cnlohr/ch32v003fun/blob/master/examples/adc_fixed_fs/adc_fixed_fs.c .
void TIM1_UP_IRQHandler(void) __attribute__((interrupt));
void TIM1_UP_IRQHandler() {
    timer_tick = 1;
    // comet_dirty = 1;

    // @todo what does this do? from source code.
    if(TIM1->INTFR & TIM_FLAG_Update) {
        TIM1->INTFR = ~TIM_FLAG_Update;
    }
}

/*
    Callback that returns a color for a LED.
    Color fornat is 0xRRGGBB.
*/
uint32_t WS2812BLEDCallback( int ledno )
{
    // @todo this will have to have a switch to handle different animations.

    uint position_within_comet = comet_position - ledno;

    if(comet_direction < 0) position_within_comet = ledno - comet_position;

    if( position_within_comet < 0 ) return (uint32_t) 0x000000;

    if( position_within_comet > (COMET_LENGTH - 1)) return (uint32_t) 0x000000;

    return comet_colors[position_within_comet];
}

void cometUpdateHandler() {
    comet_timer = comet_timer_base;

    comet_position += comet_direction;

    if( comet_position < MIN_LED ) {
        comet_position = MIN_LED;
        comet_direction = 1;
    }
    if( comet_position > LEDS ) {
        comet_position = LEDS - (COMET_LENGTH - 1);
        comet_direction = -1;
    }
    comet_dirty = 0;

    WS2812BDMAStart(LEDS);
}

int main()
{
	SystemInit();
    // Let things settle.
    Delay_Ms( 100 );

    WS2812BDMAInit();

    init_timer();

    // Let things settle.
    Delay_Ms( 100 );

    while(1)
    {
        // @todo button(s) occasional polling and debounce here.
        if ( timer_tick ) {
            timer_tick = 0;

            // if ( comet_dirty ) cometDirtyHandler(); 
            comet_timer--;

            if( comet_timer == 0 ) cometUpdateHandler();
        }
        

    }

}
