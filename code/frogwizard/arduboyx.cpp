#include <stdlib.h>
#include <avr/pgmspace.h>

#include "arduboyx.h"

ArduboyX arduboy;

static inline uint8_t bitReverse(uint8_t value) {
    uint8_t result = 0;
    asm("mov __tmp_reg__, %[in]\n\t"
        "lsl __tmp_reg__\n\t"
        "ror %[out]\n\t"
        "lsl __tmp_reg__\n\t"
        "ror %[out]\n\t"
        "lsl __tmp_reg__\n\t"
        "ror %[out]\n\t"
        "lsl __tmp_reg__\n\t"
        "ror %[out]\n\t"
        "lsl __tmp_reg__\n\t"
        "ror %[out]\n\t"
        "lsl __tmp_reg__\n\t"
        "ror %[out]\n\t"
        "lsl __tmp_reg__\n\t"
        "ror %[out]\n\t"
        "lsl __tmp_reg__\n\t"
        "ror %[out]\n\t"
        : [out] "=r"(result)
        : [in] "r"(value));

    return result;
}

void ArduboyX::drawTile(int16_t x, int16_t y, const uint8_t* bitmap, uint8_t tile, uint8_t color, bool hflip, bool vflip) {
    if(x + 8 < 0 || x >= WIDTH
    || y + 8 < 0 || y >= HEIGHT) {
        return;
    }

    bitmap += (uint16_t) tile * 8;

    uint8_t* buffer = getBuffer();
    uint8_t ofs = (y % 8 + 8) % 8;
    int16_t row = y / 8;
    
    if(y < 0) {
        --row;
    }
    
    if(row > HEIGHT / 8 - 1) {
        return;
    }

    if(row > -2) {
        for(int16_t i = 0; i != 8; ++i) {
            if(x + i >= 0 && x + i < WIDTH) {
                uint8_t a = pgm_read_byte(bitmap + (hflip ? 7 - i : i));
                if(vflip) {
                    a = bitReverse(a);
                }
                if (row >= 0) {
                    uint8_t b = a << ofs;
                    uint8_t* dest = buffer + row * WIDTH + x + i;
                    if(color == WHITE) {
                        *dest |= b;
                    } else if(color == BLACK) {
                        *dest &= ~b;
                    } else {
                        *dest ^= b;
                    }
                }
                if (ofs != 0 && -1 <= row && row < HEIGHT / 8 - 1) {
                    uint8_t b = a >> (8 - ofs);
                    uint8_t* dest = buffer + (row + 1) * WIDTH + x + i;
                    if(color == WHITE) {
                        *dest |= b;
                    } else if(color == BLACK) {
                        *dest &= ~b;
                    } else {
                        *dest ^= b;
                    }
                }
            }
        }
    }
}

void ArduboyX::printString(int16_t x, int16_t y, const uint8_t* bitmap, const char* message, uint8_t color) {
    int16_t start_x = x;
    char c;
    do {
        c = *message++;
        if(c == '\n') {
            y += 8;
            x = start_x;
        } else if(c >= 32 && c < 127) {
            drawTile(x, y, bitmap, (uint8_t) c - 32, color, false, false);
            x += 8;
        }
    } while(c != 0);
}

void ArduboyX::printProgramString(int x, int y, const uint8_t* bitmap, const char* message, uint8_t color) {
    int start_x = x;
    char c;
    do {
        c = (char) pgm_read_byte(message++);
        if(c == '\n') {
            y += 8;
            x = start_x;
        } else if(c >= 32 && c < 127) {
            drawTile(x, y, bitmap, (uint8_t) c - 32, color, false, false);
            x += 8;
        }
    } while(c != 0);
}
