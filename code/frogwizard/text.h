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

uint8_t textLength(TextType text);
void textPrint(int16_t x, int16_t y, TextType text, uint8_t color);
void textPrintCenter(int16_t x, int16_t y, TextType text, uint8_t color);
void textPrintRight(int16_t x, int16_t y, TextType text, uint8_t color);

#endif
