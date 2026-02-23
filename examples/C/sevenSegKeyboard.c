#include "ripes_system.h"

#define N 4

volatile unsigned *seg_base = SEVEN_SEGMENT_0_BASE;
volatile unsigned *kbd_base = KEYBOARD_0_BASE;

const unsigned char seg_digit[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

const unsigned char seg_alpha[26] = {
    0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x06, 0x1E,
    0x76, 0x38, 0x37, 0x54, 0x3F, 0x73, 0x67, 0x50, 0x6D, 0x78,
    0x3E, 0x1C, 0x3E, 0x76, 0x6E, 0x5B
};

unsigned char encode(unsigned char ch) {
    if (ch >= '0' && ch <= '9') return seg_digit[ch - '0'];
    if (ch >= 'A' && ch <= 'Z') return seg_alpha[ch - 'A'];
    return 0;
}

void main() {
    int pos = 0;
    int i;

    for (i = 0; i < N; i++)
        *(seg_base + i) = 0;

    while (1) {
        if (*(kbd_base + 1) == 0)
            continue;

        unsigned char key = (unsigned char)(*(kbd_base));

        unsigned char seg = encode(key);
        if (seg) {
            *(seg_base + pos) = seg;
            pos++;
            if (pos >= N)
                pos = 0;
        }
    }
}