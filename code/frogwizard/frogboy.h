#ifndef FROGBOY_H
#define FROGBOY_H

#include <stdint.h>

#define FROGBOY_APPNAME "frog wizard"

namespace frogboy {
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
        BUTTON_PAUSE,
        BUTTON_RESET,

        BUTTON_COUNT,
    };

    enum ButtonMask {
        BUTTON_MASK_LEFT = 1 << BUTTON_LEFT,
        BUTTON_MASK_RIGHT = 1 << BUTTON_RIGHT,
        BUTTON_MASK_UP = 1 << BUTTON_UP,
        BUTTON_MASK_DOWN = 1 << BUTTON_DOWN,
        BUTTON_MASK_SHOOT = 1 << BUTTON_SHOOT,
        BUTTON_MASK_JUMP = 1 << BUTTON_JUMP,
        BUTTON_MASK_PAUSE = 1 << BUTTON_PAUSE,
        BUTTON_MASK_RESET = 1 << BUTTON_RESET,
    };

    bool init();
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
    void playTone(uint16_t frequency, uint32_t duration);
    bool isPressed(Button code);
    bool anyPressed(uint8_t buttonMask);
    int getRandom(int min, int max);

    template<typename T> T readRom(const T* ptr);
}

#ifdef __AVR__
    #include <avr/pgmspace.h>

    #define FROGBOY_ROM_DATA PROGMEM

    namespace frogboy {
        template<typename T> T readRom(const T* ptr) {
            return reinterpret_cast<T>(pgm_read_word(ptr));
        }        
        template<> char readRom(const char* ptr);
        template<> uint8_t readRom(const uint8_t* ptr);
        template<> int8_t readRom(const int8_t* ptr);
        template<> bool readRom(const bool* ptr);
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
