#ifndef TEXT_H
#define TEXT_H

enum TextType {
    TEXT_TYPE_TITLE,
    TEXT_TYPE_PRESS_START,
    TEXT_TYPE_AUTHOR,
    TEXT_TYPE_PAUSED,
    TEXT_TYPE_FIND_EGG,

    TEXT_TYPE_COUNT,
};

namespace text {
    uint8_t length(TextType text);
    void print(int16_t x, int16_t y, TextType text, uint8_t color);
    void printCenter(int16_t x, int16_t y, TextType text, uint8_t color);
    void printRight(int16_t x, int16_t y, TextType text, uint8_t color);
}

#endif
