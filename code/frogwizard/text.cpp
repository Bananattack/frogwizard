#include "frogboy.h"
#include "sprites_bitmap.h"
#include "text.h"

namespace {
    const char titleText[] FROGBOY_ROM_DATA = "F R O G  E G G";
    const char pressStartText[] FROGBOY_ROM_DATA = "PRESS START";
    const char authorText[] FROGBOY_ROM_DATA = "BY EGGBOYCOLOR";
    const char pausedText[] FROGBOY_ROM_DATA = "PAUSED";
    const char findEggText[] FROGBOY_ROM_DATA = " COLLECT\n EGGS.";

    const char* const textPointers[TEXT_TYPE_COUNT] FROGBOY_ROM_DATA = {
        titleText,
        pressStartText,
        authorText,
        pausedText,
        findEggText,
    };

    const uint8_t textLengths[TEXT_TYPE_COUNT] FROGBOY_ROM_DATA = {
        sizeof(titleText) - 1,
        sizeof(pressStartText) - 1,
        sizeof(authorText) - 1,
        sizeof(pausedText) - 1,
        sizeof(findEggText) - 1,
    };
}

namespace text {
    uint8_t length(TextType text) {
        return text < TEXT_TYPE_COUNT ? frogboy::readRom<uint8_t>(&textLengths[text]) : 0;
    }

    void print(int16_t x, int16_t y, TextType text, uint8_t color) {
        if(text < TEXT_TYPE_COUNT) {
            frogboy::printRomString(x, y, spritesBitmap, frogboy::readRom<const char*>(&textPointers[text]), color);
        }
    }

    void printCenter(int16_t x, int16_t y, TextType text, uint8_t color) {
        print(x - length(text) / 2 * 8, y, text, color);
    }

    void printRight(int16_t x, int16_t y, TextType text, uint8_t color) {
        print(x - length(text) * 8 - 8, y, text, color);
    }
}