#ifdef __AVR__
#include <stdlib.h>
#include <Arduino.h>
#include <Arduboy.h>

#include "frogboy.h"

namespace {
    Arduboy arduboy;

    const uint8_t keycodes[static_cast<size_t>(frogboy::BUTTON_COUNT)] FROGBOY_ROM_DATA = {
        LEFT_BUTTON,
        RIGHT_BUTTON,
        UP_BUTTON,
        DOWN_BUTTON,        
        A_BUTTON,
        B_BUTTON,
        DOWN_BUTTON,
        LEFT_BUTTON | RIGHT_BUTTON | UP_BUTTON | DOWN_BUTTON | A_BUTTON | B_BUTTON,
    };
}

namespace frogboy {
    bool init() {
        arduboy.begin(); 
        arduboy.setFrameRate(60);
        arduboy.initRandomSeed();
        return true;
    }

    void destroy() {

    }

    void clearScreen() {
        arduboy.clear();
    }

    void refreshScreen() {
        arduboy.display();
    }

    bool waitForFrame() {
        return arduboy.nextFrame();
    }

    bool isActive() {
        return true;
    }

    uint8_t reverseBits(uint8_t value) {
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

    uint8_t* getScreenBuffer() {
        return arduboy.getBuffer();
    }

    void playTone(uint16_t frequency, uint32_t duration) {
        arduboy.tunes.tone(frequency, duration);
    }

    void playMusic(const uint8_t* data) {
        arduboy.tunes.playScore(data);
    }

    void stopMusic() {
        arduboy.tunes.stopScore();        
    }

    bool isMusicPlaying() {
        return arduboy.tunes.playing();
    }

    bool isPressed(Button button) {
        return arduboy.pressed(frogboy::readRom<uint8_t>(keycodes + static_cast<uint8_t>(button)));
    }

    int getRandom(int min, int max) {
        return random(min, max + 1);
    }

    struct Save {
        bool open;
        int address;
    };

    Save saveHandle;

    Save* saveOpen(SaveOpenMode mode) {
        if(!saveHandle.open) {
            saveHandle.open = true;
            saveHandle.address = EEPROM_STORAGE_SPACE_START;
            return &saveHandle;
        }

        return nullptr;        
    }

    void saveReadBytes(Save* save, uint16_t length, void* data) {
        if(data != nullptr) {
            char* ptr = static_cast<char*>(data);
            while(length != 0) {
                *ptr++ = EEPROM.read(saveHandle.address++);
                length--;
            }
        } else {
            saveHandle.address += length;
        }
    }

    void saveWriteBytes(Save* save, uint16_t length, const void* data) {
        const char* ptr = static_cast<const char*>(data);
        while(length != 0) {
            EEPROM.write(saveHandle.address++, *ptr++);
            length--;
        }
    }
        
    void saveClose(Save* save) {
        saveHandle.open = false;
    }

    template<> char readRom(const char* ptr) {
        return static_cast<char>(pgm_read_byte(ptr));
    }

    template<> uint8_t readRom(const uint8_t* ptr) {
        return pgm_read_byte(ptr);
    }

    template<> int8_t readRom(const int8_t* ptr) {
        return static_cast<int8_t>(pgm_read_byte(ptr));
    }

    template<> bool readRom(const bool* ptr) {
        return static_cast<bool>(pgm_read_byte(ptr));
    }        
}
#endif