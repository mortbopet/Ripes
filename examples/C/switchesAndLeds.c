#define W 35
#define H 25

unsigned* led_base = 0xF0000004;
unsigned* switch_base = 0xF0000000;

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
