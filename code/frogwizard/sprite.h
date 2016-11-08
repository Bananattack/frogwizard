#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>

enum SpriteType {
    SPRITE_TYPE_PLAYER_1,
    SPRITE_TYPE_PLAYER_2,
    SPRITE_TYPE_FIREBALL_1,
    SPRITE_TYPE_FIREBALL_2,
    SPRITE_TYPE_CIRCLE,
    SPRITE_TYPE_WALKER_1,
    SPRITE_TYPE_WALKER_2,
    SPRITE_TYPE_DOOR,
    SPRITE_TYPE_COUNT,
};

enum SpriteFlags {
    SPRITE_FLAG_COLOR_INVERT = 0x01,
    SPRITE_FLAG_HFLIP = 0x40,
    SPRITE_FLAG_VFLIP = 0x80,
};

void spriteDraw(int16_t x, int16_t y, SpriteType frame, uint8_t flags);
#endif
