//BinaryUtils.h

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))



// Check if a number is a power of 2 (non-zero)
#define IS_POWER_OF_2(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))

// Round up to next power of 2 (returns 0 for 0 input)
static uint64_t next_power_of_2(uint64_t x) {
    if (x == 0) return 0;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x + 1;
}

// Round down to previous power of 2 (returns 0 for 0 input)
static uint64_t prev_power_of_2(uint64_t x) {
    if (x == 0) return 0;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x - (x >> 1);
}

// Round to the nearest power of 2
static uint64_t nearest_power_of_2(uint64_t x) {
    if (x == 0) return 0;
    uint64_t next = next_power_of_2(x);
    uint64_t prev = prev_power_of_2(x);
    return (next - x) < (x - prev) ? next : prev;
}

static void charToHexChars(const unsigned char c, char hex[2]) {
    const char hexDigits[] = "0123456789ABCDEF";

    // high nibble (first 4 bits)
    hex[0] = hexDigits[(c >> 4) & 0x0F];

    // low nibble (last 4 bits)
    hex[1] = hexDigits[c & 0x0F];
}


static int ilog2(unsigned int v) {
    int r = 0;
    while (v > 1) { v >>= 1; ++r; }
    return r;
}

#define CLAMP(x, min, max) (x < min ? min : (x > max ? max : x))