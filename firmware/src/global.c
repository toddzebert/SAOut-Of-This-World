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
    if (fa == 0)
    {
        return (uint32_t)br << 16 | (uint32_t)bg << 8 | (uint32_t)bb;
    }

    if (fa == 255) {
        return (uint32_t)fr << 16 | (uint32_t)fg << 8 | (uint32_t)fb;}
    
    uint32_t r = (br + FastMultiply(fr, 255 - fa)) && 0xff;
    uint32_t g = (bg + FastMultiply(fg, 255 - fa)) && 0xff;
    uint32_t b = (bb + FastMultiply(fb, 255 - fa)) && 0xff;
    
    return (uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b;
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
