#include "ripes_system.h"

/* Switches and LEDs
 * This program will toggle an LED in an LED matrix when the corresponding
 * switch is toggled.
 *
 * To run this program, make sure that you have instantiated one of the
 * following peripherals in the I/O tab:
 * - "LED Matrix"
 * - "Switches"
 */

unsigned* led_base = LED_MATRIX_0_BASE;
unsigned* switch_base = SWITCHES_0_BASE;

void main() {
    unsigned v = 0;
    while(1) {
		for(int i = 0; i < 8; i++) {
            if((*switch_base >> i) & 0x1) {
                *(led_base + i) = 0xFFFFFF;
            } else {
                *(led_base + i) = 0x0;
            }
        }
    }
}
