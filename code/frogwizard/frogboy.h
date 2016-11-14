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

    enum MusicOpcode {
        // Wait: wait for the specified number of milliseconds
        // duration = ((cmd << 8) | next), cmd in {0x00 .. 0x7F}, next in {0x00 .. 0xFF}
        MUSIC_OPCODE_WAIT = 0x00,

        // Stop note: stop any playing note on the specified channel.
        // channel = (cmd & 0xF)
        MUSIC_OPCODE_STOP_NOTE = 0x80,

        // Play note: play a new note on the specified channel.
        // channel = (cmd & 0xF)
        // note = next, next in {0x00 .. 0x7F}
        MUSIC_OPCODE_PLAY_NOTE = 0x90,

        // Restart: Sends the music player back to the begining of the sequence.
        MUSIC_OPCODE_RESTART = 0xE0,

        // Stop: Stops the music sequence.
        MUSIC_OPCODE_STOP = 0xF0,
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
    void playMusic(const uint8_t* data);
    void stopMusic();
    bool isMusicPlaying();
    bool isPressed(Button code);
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
