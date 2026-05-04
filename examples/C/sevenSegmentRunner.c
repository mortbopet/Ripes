#include "ripes_system.h"

/*
 * sevenSegmentRunner.c
 * Slides "67" across the seven-segment display.
 */

#define N             SEVEN_SEGMENT_0_N_DIGITS
#define DELAY_LOOPS   2000000u

#define SEG_6 0x7D
#define SEG_7 0x07

static volatile unsigned *const seg = SEVEN_SEGMENT_0_BASE;

static void delay(void) {
    volatile unsigned i;
    for (i = 0; i < DELAY_LOOPS; ++i) {
    }
}

int main(void) {
    int pos = 0;
    while (1) {
        for (int i = 0; i < N; ++i)
            seg[i] = 0;

        seg[pos] = SEG_6;
        if (pos + 1 < N)
            seg[pos + 1] = SEG_7;

        delay();
        pos = (pos + 1) % N;
    }
    return 0;
}
