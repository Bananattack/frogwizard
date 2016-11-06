#include "frogboy.h"

namespace frogboy {
    void drawTile(int16_t x, int16_t y, const uint8_t* bitmap, uint8_t tile, uint8_t color, bool hflip, bool vflip) {
        if(x + 8 < 0 || x >= SCREEN_WIDTH
        || y + 8 < 0 || y >= SCREEN_HEIGHT) {
            return;
        }

        bitmap += (uint16_t) tile * 8;

        uint8_t* buffer = getScreenBuffer();
        uint8_t ofs = (y % 8 + 8) % 8;
        int16_t row = y / 8;
        
        if(y < 0) {
            --row;
        }
        
        if(row > SCREEN_HEIGHT / 8 - 1) {
            return;
        }

        if(row > -2) {
            for(int16_t i = 0; i != 8; ++i) {
                if(x + i >= 0 && x + i < SCREEN_WIDTH) {
                    uint8_t a = readRom<uint8_t>(bitmap + (hflip ? 7 - i : i));
                    if(vflip) {
                        a = reverseBits(a);
                    }
                    if (row >= 0) {
                        uint8_t b = a << ofs;
                        uint8_t* dest = buffer + row * SCREEN_WIDTH + x + i;
                        if(color == 0) {
                            *dest &= ~b;
                        } else if(color == 1) {
                            *dest |= b;
                        } else {
                            *dest ^= b;
                        }
                    }
                    if (ofs != 0 && -1 <= row && row < SCREEN_HEIGHT / 8 - 1) {
                        uint8_t b = a >> (8 - ofs);
                        uint8_t* dest = buffer + (row + 1) * SCREEN_WIDTH + x + i;
                        if(color == 0) {
                            *dest &= ~b;
                        } else if(color == 1) {
                            *dest |= b;
                        } else {
                            *dest ^= b;
                        }
                    }
                }
            }
        }
    }

    void printRamString(int16_t x, int16_t y, const uint8_t* bitmap, const char* message, uint8_t color) {
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

    void printRomString(int16_t x, int16_t y, const uint8_t* bitmap, const char* message, uint8_t color)  {
        int16_t start_x = x;
        char c;
        do {
            c = readRom<char>(message++);
            if(c == '\n') {
                y += 8;
                x = start_x;
            } else if(c >= 32 && c < 127) {
                drawTile(x, y, bitmap, (uint8_t) c - 32, color, false, false);
                x += 8;
            }
        } while(c != 0);
    }
}
