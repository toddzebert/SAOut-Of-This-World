#include "global.h"

volatile uint8_t registry[140] = {0};

uint16_t thing_timer[THING_COUNT];

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
