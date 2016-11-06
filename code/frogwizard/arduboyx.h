#ifndef ARDUBOY_X_H
#define ARDUBOY_X_H

#include <Arduboy.h>

class ArduboyX : public Arduboy {
    public:
        void drawTile(int16_t x, int16_t y, const uint8_t* bitmap, uint8_t tile, uint8_t color, bool hflip, bool vflip);
        void printString(int16_t x, int16_t y, const uint8_t* bitmap, const char* message, uint8_t color);
        void printProgramString(int16_t x, int16_t y, const uint8_t* bitmap, const char* message, uint8_t color);
};

extern ArduboyX arduboy;

#endif
