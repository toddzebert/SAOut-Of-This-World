#include "global.h"

#include <color_utilities.h>

volatile uint8_t registry[140] = {0};

uint16_t thing_tock_timer[THING_COUNT] = {1, 1, 1, 1};

const uint8_t thing_led_count[] = {
    STARS_COUNT,
    EYES_COUNT,
    UPPER_TRIM_COUNT,
    LOWER_TRIM_COUNT
};

uint16_t thing_tock_timer[THING_COUNT];

State_Action_t state_action[THING_COUNT];

const Event_t Event_None = {
    .type = EVENT_NONE,
    .thing = THING_NONE
};

const uint8_t reg_thing_start[THING_COUNT] = {
    REG_STARS_START,
    REG_EYES_START,
    REG_UPPER_TRIM_START,
    REG_LOWER_TRIM_START
};

const uint8_t reg_thing_led_start[THING_COUNT] = {
    REG_STARS_LED_START,
    REG_EYES_LED_START,
    REG_UPPER_TRIM_LED_START,
    REG_LOWER_TRIM_LED_START
};

const uint8_t RGB_Black[3] = {0, 0, 0};

#define EVENT_QUEUE_SIZE 16
volatile Event_t Event_queue[EVENT_QUEUE_SIZE];
volatile uint8_t Event_Queue_head = EVENT_QUEUE_SIZE - 1;
volatile uint8_t Event_Queue_tail = EVENT_QUEUE_SIZE - 1;
volatile uint8_t Event_Queue_count = 0;

/**
 * Checks if the event queue is empty.
 *
 * @return true if the event queue is empty, false otherwise.
 */
bool eventQueueEmpty() {
    // printf("EventQueueEmpty, count: %d\n", Event_Queue_count); // @debug
    return Event_Queue_count == 0;
}

/**
 * Checks if the event queue is full.
 *
 * @return true if the event queue is full, false otherwise.
 */
bool eventQueueFull() {
    printf("EventQueueFull, count: %d\n", Event_Queue_count); // @debug
    return Event_Queue_count == EVENT_QUEUE_SIZE;
}

/**
 * Adds an event to the event queue if there is space available.
 *
 * @param event The event to be added to the queue.
 * @return true if the event was successfully added, false if the queue is full.
 */
bool eventPush(Event_t event) {
    printf( "In eventPush, event.type %d, thing %d\r\n", event.type, event.thing ); // @debug
    if (eventQueueFull()) return false;

    // Add Event and decrement after.
    Event_queue[Event_Queue_head--] = event;
    Event_Queue_count++;
    
    // Wrap to top if needed.
    if (Event_Queue_head == 0) Event_Queue_head = EVENT_QUEUE_SIZE - 1;

    printf("leaving eventPush\r\n"); // @debug
    return true;
}

/**
 * Removes an event from the event queue.
 *
 * @return The event at the tail of the queue if the queue is not empty; 
 *         returns Event_None if the queue is empty.
 *
 * This function checks if the event queue is empty and returns Event_None if so.
 * Otherwise, it retrieves the event at the current tail position, increments 
 * the tail index, and decreases the event queue count. The tail index wraps 
 * around to the beginning of the queue if it reaches the end.
 */
Event_t eventPop() {
    printf( "In eventPop\r\n" ); // @debug

    if (eventQueueEmpty()) return Event_None;

    // Get Event and increment after.
    Event_t event = Event_queue[Event_Queue_tail++];
    Event_Queue_count--;
    
    // Wrap to top if needed.
    if (Event_Queue_tail == EVENT_QUEUE_SIZE) Event_Queue_tail = 0;

    printf( "Leaving eventPop, event.type %d, thing %d\r\n", event.type, event.thing ); // @debug

    return event;
}

/**
 * @brief Copy from array to registry.
 *
 * @param dest Destination. Must be a pointer to volatile uint8_t.
 * @param dest_offset Offset from start of dest to start copying to.
 * @param src Source. Must be a pointer to uint8_t.
 * @param src_offset Offset from start of src to start copying from.
 * @param dest_len Number of bytes to copy.
 *
 * @note No return value.
 * 
 * This function exists because memcpy() does not take volatile params.
 */
void arrayToRegCopy(volatile uint8_t *dest, size_t dest_offset, uint8_t *src, size_t src_offset, size_t dest_len) {
    const uint8_t *s = src;

    for (size_t i = 0; i < dest_len; i++) {
        dest[dest_offset + i] = s[src_offset + i];
    }
}

/**
 * @brief Copy from registry to array.
 *
 * @param dest Destination. Must be a pointer to uint8_t.
 * @param dest_offset Offset from start of dest to start copying to.
 * @param src Source. Must be a pointer to volatile uint8_t.
 * @param src_offset Offset from start of src to start copying from.
 * @param dest_len Number of bytes to copy.
 *
 * @note No return value.
 * 
 * This function exists because memcpy() does not take volatile params.
 */
void regToArrayCopy(uint8_t *dest, size_t dest_offset, volatile uint8_t *src, size_t src_offset, size_t dest_len)
{
    volatile uint8_t *s = src;

    for (size_t i = 0; i < dest_len; i++) {
        dest[dest_offset + i] = s[src_offset + i];
    }
}

/**
 * @brief Copy from const array to registry.
 *
 * @param dest Destination. Must be a pointer to volatile uint8_t.
 * @param dest_offset Offset from start of dest to start copying to.
 * @param src Source. Must be a pointer to const uint8_t.
 * @param src_offset Offset from start of src to start copying from.
 * @param dest_len Number of bytes to copy.
 *
 * @note No return value.
 * 
 * This function exists because memcpy() does not take volatile params.
 */
void constToRegCopy(volatile uint8_t *dest, size_t dest_offset, const uint8_t *src, size_t src_offset, size_t dest_len) {
    const uint8_t *s = src;

    for (size_t i = 0; i < dest_len; i++) {
        dest[dest_offset + i] = s[src_offset + i];
    }
}

/**
 * @brief Copy from one registry to another.
 *
 * @param dest Destination. Must be a pointer to volatile uint8_t.
 * @param dest_offset Offset from start of dest to start copying to.
 * @param src Source. Must be a pointer to volatile uint8_t.
 * @param src_offset Offset from start of src to start copying from.
 * @param dest_len Number of bytes to copy.
 *
 * @note No return value.
 * 
 * This function exists because memcpy() does not take volatile params.
 */
void regToRegCopy(volatile uint8_t *dest, size_t dest_offset, volatile uint8_t *src, size_t src_offset, size_t dest_len) {
    volatile uint8_t *s = src;

    for (size_t i = 0; i < dest_len; i++) {
        dest[dest_offset + i] = s[src_offset + i];
    }
}

// @note Compilier attribute forces the RISC compiler to inline this function, otherwise inline is just a suggestion.
inline uint8_t byteIsPowerOfTwo(uint8_t x) __attribute__((always_inline));
/**
 * @brief Returns true if the byte is a power of two.
 *
 * @param x Byte to check.
 *
 * @returns Non-zero if the byte is a power of two, zero otherwise.
 *
 * @note This function is inlined because it's a simple and important check.
 */
inline uint8_t byteIsPowerOfTwo(uint8_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
    // @note or return ((x != 0) && ((x & (~x + 1)) == x));
    // @note or return (x == 1 || x == 2 || x == 4 || x == 8 || x == 16 || x == 32 || x == 64 || x == 128);
}

// @note see (used here) premultiplied https://en.wikipedia.org/wiki/Alpha_compositing#Straight_versus_premultiplied .
// @note for "straight" see https://stackoverflow.com/questions/726549/algorithm-for-additive-color-mixing-for-rgb-values .
// @note this can easily clip the result.
u_int32_t blendHexColorsWithAlpha(uint8_t br, uint8_t bg, uint8_t bb, uint8_t fr, uint8_t fg, uint8_t fb, uint8_t fa)
{
    // printf("blendHexColorsWithAlpha: %d %d %d %d %d %d %d\n", br, bg, bb, fr, fg, fb, fa); // @debug

    if (fa == 0) return (uint32_t)br << 16 | (uint32_t)bg << 8 | (uint32_t)bb; // Return background.

    if (fa == 255) return (uint32_t)fr << 16 | (uint32_t)fg << 8 | (uint32_t)fb; // Return foreground.
    
    /* @debug all below
    uint32_t temp = FastMultiply(fr, 255 - fa);
    printf("RED FastMultiply(fr, 255 - fa): %lu\n", temp); // @debug
    temp = temp >> 8;
    printf("RED temp >> 8: %lu\n", temp); // @debug
    temp = temp + br;
    printf("RED temp + br: %lu\n", temp); // @debug
    temp = temp & 0xff;
    printf("RED temp & 0xff: %lu\n", temp); // @debug

    temp = FastMultiply(fg, 255 - fa);
    printf("GREEN FastMultiply(fg, 255 - fa): %lu\n", temp); // @debug
    temp = temp >> 8;
    printf("GREEN temp >> 8: %lu\n", temp); // @debug
    temp = temp + bg;
    printf("GREEN temp + bg: %lu\n", temp); // @debug
    temp = temp & 0xff;
    printf("GREEN temp & 0xff: %lu\n", temp); // @debug
    @debug end. */

    /* @debug works, basically 
    uint32_t r = (br + (FastMultiply(fr, 255 - fa) >> 8)) & 0xff;
    uint32_t g = (bg + (FastMultiply(fg, 255 - fa) >> 8)) & 0xff;
    uint32_t b = (bb + (FastMultiply(fb, 255 - fa) >> 8)) & 0xff;
    */

    /* @debug like color's tween but doesn't work
    uint32_t r = (br + (FastMultiply(fr, 255 - fa) + 128)) >> 8;
    uint32_t g = (bg + (FastMultiply(fg, 255 - fa) + 128)) >> 8;
    uint32_t b = (bb + (FastMultiply(fb, 255 - fa) + 128)) >> 8;
    */

    // @debug
    uint32_t r = (br + ((FastMultiply(fr, 255 - fa) + 128) >> 8)) & 0xff;
    uint32_t g = (bg + ((FastMultiply(fg, 255 - fa) + 128) >> 8)) & 0xff;
    uint32_t b = (bb + ((FastMultiply(fb, 255 - fa) + 128) >> 8)) & 0xff;

    // printf("blendHexColorsWithAlpha: %lu %lu %lu\n", r, g, b); // @debug
    
    return (r << 16) | (g << 8) | b;
}

/**
 * @brief Print out non-zero values in the registry.
 *
 * @param reg Pointer to start of registry.
 *
 * @note No return value.
 * 
 * This (debug) function prints out the register address and value for all non-zero registry values.
 */
void printNon0Reg(volatile uint8_t *reg) {
    volatile uint8_t *s = reg;

    for (int i = 0; i < REG_COUNT; i++) {
        if (s[i])
        {
            printf("%d:0x%02x ", i, s[i]);
        }
    }

    printf("\n");
}


/**
 * @brief Print out a byte in binary form.
 *
 * @param c The byte to print.
 * @param newline Print a newline after printing, or not.
 *
 * @note No return value.
 * 
 * This (debug) function prints out the 8 bits of the given byte in binary form, with
 * the leftmost (most significant) bit first. If @p newline is true, a newline is printed
 * after printing the binary value.
 */
void printBin(uint8_t c, int newline)
{
    for (int i = 7; i >= 0; --i)
    {
        putchar( (c & (1 << i)) ? '1' : '0' );
    }

    if (newline) putchar('\n');
}


/**
 * @brief Print out a byte in binary form, referenced by a pointer.
 *
 * @param pnt0 Pointer to the byte to print.
 * @param newline Print a newline after printing, or not.
 *
 * @note No return value.
 * 
 * This (debug) function prints out the 8 bits of the given byte in binary form, with
 * the leftmost (most significant) bit first. If @p newline is true, a newline is printed
 * after printing the binary value.
 */
void printBinByRef(void *pnt0, int newline)
{
    unsigned char *pnt = pnt0;
    uint8_t size = 8;

    while (size--) {
        printf("%02x", *pnt++);
    }

    if (newline) putchar('\n');
}
