#include "ripes_system.h"

/* LEDs
 * This program draws an animation on an LED matrix peripheral.
 *
 * To run this program, make sure that you have instantiated an "LED Matrix"
 * peripheral in the "I/O" tab.
 */

#define W LED_MATRIX_0_WIDTH
#define H LED_MATRIX_0_HEIGHT
unsigned* led_base = LED_MATRIX_0_BASE;

void main() {
    unsigned v = 0;
    while (1) {
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                char r = v + (0xFF * x) / W;
                char g = v + (0xFF * y) / H;
                char b = v + (0xFF * (x + y)) / (W + H);
                unsigned idx = y * W + x;
                *(led_base + idx) = r << 16 | g << 8 | b;
            }
            v++;
        }
    }
}
