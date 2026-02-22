#include "ripes_system.h"

/* Mouse Drawing
 * This program demonstrates the mouse peripheral by drawing on an LED matrix.
 *
 * - Hold left mouse button to draw along the cursor path
 * - Hold right mouse button to erase around the cursor
 * - Scroll the mouse wheel to cycle the drawing color (R -> G -> B)
 *
 * To run this program, make sure that you have instantiated one of each of the
 * following peripherals in the I/O tab:
 * - "LED Matrix"
 * - "Mouse"
 * 
 * created by Aleksandr Galkin https://github.com//t33nsy 
 */

#define W LED_MATRIX_0_WIDTH
#define H LED_MATRIX_0_HEIGHT
#define ERASE_RADIUS 5

unsigned int *led_base = LED_MATRIX_0_BASE;
volatile unsigned int *mouse_base = MOUSE_0_BASE;

// Function for colouring one pixel
void set_pixel(int x, int y, unsigned int color) {
    if (x >= 0 && x < W && y >= 0 && y < H)
        *(led_base + y * W + x) = color;
}

// Absolute value func for not including math
int abs(int num){
    int ans = num < 0 ? -num : num;
    return num;
}

// Bresenham's line algorithm
void draw_line(int x0, int y0, int x1, int y1, unsigned int color) {
    int deltaX = abs(x1 - x0);
    int deltaY = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = deltaX - deltaY;

    while (x0 != x1 || y0 != y1) {
        set_pixel(x0, y0, color);
        int e2 = err << 1;
        if (e2 > -deltaY) {
            err -= deltaY;
            x0 += sx;
        }
        if (e2 < deltaX) {
            err += deltaX;
            y0 += sy;
        }
    }
    set_pixel(x1, y1, color);
}

// Erase function (colouring black with radius)
void erase_area(int cx, int cy, int radius) {
    for (int deltaY = -radius; deltaY <= radius; ++deltaY)
        for (int deltaX = -radius; deltaX <= radius; ++deltaX)
            if (deltaX * deltaX + deltaY * deltaY <= radius * radius)
                set_pixel(cx + deltaX, cy + deltaY, 0x000000);
}

int main() {
    /* 
     * colors[0] = red   255 0   0
     * colors[1] = green 0   255 0
     * colors[2] = blue  0   0   255
     */
    unsigned colors[] = {0xFF0000, 0x00FF00, 0x0000FF};
    int color_idx = 0, was_drawing = 0;
    int prev_scroll = 0, prev_x = -1, prev_y = -1;

    // Clear the LED matrix
    for (int i = 0; i < W * H; ++i)
        *(led_base + i) = 0x000000;

    while (1) {
        // Read mouse registers
        int mouseX = *(mouse_base + 0);        /* X position   */
        int mouseY = *(mouse_base + 1);        /* Y position   */
        int leftButton = *(mouse_base + 2);    /* Left button  */
        int rightButton = *(mouse_base + 3);   /* Right button */
        int scroll = *(mouse_base + 4);        /* Scroll wheel */

        // Scale mouse coordinates to LED matrix
        int ledX = mouseX * W / MOUSE_0_WIDTH;
        int ledY = mouseY * H / MOUSE_0_HEIGHT;
        ledX = ledX >= W ? W - 1: ledX;
        ledY = ledY >= H ? H - 1: ledY;

        // Scroll changes color by ring: R -> G -> B
        if (scroll != prev_scroll) {
            if (scroll > prev_scroll)
                color_idx = (color_idx + 1) % 3;
            else
                color_idx = (color_idx + 2) % 3;
            prev_scroll = scroll;
        }

        // Left button: draw line with cursor trajectory
        if (leftButton) {
            if (was_drawing && prev_x >= 0)
                draw_line(prev_x, prev_y, ledX, ledY, colors[color_idx]);
            else
                set_pixel(ledX, ledY, colors[color_idx]);
            prev_x = ledX;
            prev_y = ledY;
            was_drawing = 1;
        } else
            was_drawing = 0;

        // Right button: erase area with radius around cursor
        if (rightButton)
            erase_area(ledX, ledY, ERASE_RADIUS);
    }
    return 0;
}