#define W 35
#define H 25

unsigned* led_base = 0xF0000000;

void main() {
    unsigned v = 0;
    while(1) {
        for(int y = 0; y < H; y++) {
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