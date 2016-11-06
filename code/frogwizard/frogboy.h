#ifndef FROGBOY_H
#define FROGBOY_H

#include <stdint.h>

#define FROGBOY_APPNAME "frog wizard"

namespace frogboy {
    void init();
    void destroy();
    void clearScreen();
    void refreshScreen();
    bool waitForFrame();
    bool isActive();
    uint8_t reverseBits(uint8_t value);
    uint8_t* getScreenBuffer();
    void drawTile(int16_t x, int16_t y, const uint8_t* bitmap, uint8_t tile, uint8_t color, bool hflip, bool vflip);
    void printRamString(int16_t x, int16_t y, const uint8_t* bitmap, const char* message, uint8_t color);
    void printRomString(int16_t x, int16_t y, const uint8_t* bitmap, const char* message, uint8_t color);

    enum {
        SCREEN_WIDTH = 128,
        SCREEN_HEIGHT = 64,
    };

    enum Button {
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_SHOOT,
        BUTTON_JUMP,

        BUTTON_COUNT,
    };

    bool isPressed(Button code);
    int getRandom(int min, int max);

    template<typename T> T readRom(const T* ptr);
}

#ifdef __AVR__
    #include <avr/pgmspace.h>

    #define FROGBOY_ROM_DATA PROGMEM

    namespace frogboy {
        template<typename T> T readRom(const T* ptr) {
            return static_cast<T>(pgm_read_word(ptr));
        }    
        template<> uint8_t readRom(const uint8_t* ptr) {
            return pgm_read_byte(ptr);
        }
        template<> uint8_t readRom(const int8_t* ptr) {
            return static_cast<int8_t>(pgm_read_byte(ptr));
        }
    }
#else
    #define FROGBOY_ROM_DATA

    namespace frogboy {
        template<typename T> T readRom(const T* ptr) {
            return *ptr;
        }
    }
#endif

#endif
