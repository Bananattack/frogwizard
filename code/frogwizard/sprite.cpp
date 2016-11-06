#include <avr/pgmspace.h>

#include "arduboyx.h"
#include "sprite.h"
#include "sprites_bitmap.h"



enum {
    SPRITE_FIELD_LENGTH,
    SPRITE_FIELD_WIDTH,
    SPRITE_FIELD_HEIGHT,
    SPRITE_FIELD_SPRITES,
};

enum {
    SPRITE_SPRITE_FIELD_X,
    SPRITE_SPRITE_FIELD_Y,
    SPRITE_SPRITE_FIELD_ATTR,
    SPRITE_SPRITE_FIELD_TILE,
};

enum {
    SPRITE_ATTR_HFLIP = SPRITE_FLAG_HFLIP,
    SPRITE_ATTR_VFLIP = SPRITE_FLAG_VFLIP,    
};



const uint8_t playerSprite1[] PROGMEM = {
    0x04, 0x10, 0x10,
    0x00, 0x00, (uint8_t) 0x00, 0x66,
    0x08, 0x00, (uint8_t) 0x00, 0x67,
    0x00, 0x08, (uint8_t) 0x00, 0x76,
    0x08, 0x08, (uint8_t) 0x00, 0x77,    
};

const uint8_t playerSprite2[] PROGMEM = {
    0x04, 0x10, 0x10,
    0x00, 0x00, (uint8_t) 0x00, 0x68,
    0x08, 0x00, (uint8_t) 0x00, 0x69,
    0x00, 0x08, (uint8_t) 0x00, 0x78,
    0x08, 0x08, (uint8_t) 0x00, 0x79,    
};

const uint8_t fireballSprite1[] PROGMEM = {
    0x04, 0x10, 0x10,
    0x00, 0x00, (uint8_t) 0x00, 0x80,
    0x08, 0x00, (uint8_t) 0x00, 0x81,
    0x00, 0x08, (uint8_t) 0x00, 0x90,
    0x08, 0x08, (uint8_t) 0x00, 0x91,    
};

const uint8_t fireballSprite2[] PROGMEM = {
    0x04, 0x10, 0x10,
    0x00, 0x00, (uint8_t) 0x00, 0x82,
    0x08, 0x00, (uint8_t) 0x00, 0x83,
    0x00, 0x08, (uint8_t) 0x00, 0x92,
    0x08, 0x08, (uint8_t) 0x00, 0x93,    
};

const uint8_t circleSprite1[] PROGMEM = {
    0x04, 0x10, 0x10,
    0x00, 0x00, (uint8_t) 0x00, 0x84,
    0x08, 0x00, (uint8_t) SPRITE_ATTR_HFLIP, 0x84,
    0x00, 0x08, (uint8_t) SPRITE_ATTR_VFLIP, 0x84,
    0x08, 0x08, (uint8_t) SPRITE_ATTR_HFLIP | SPRITE_ATTR_VFLIP, 0x84,
};

const uint8_t walkerSprite1[] PROGMEM = {
    0x04, 0x10, 0x10,
    0x00, 0x00, (uint8_t) 0x00, 0x86,
    0x08, 0x00, (uint8_t) 0x00, 0x87,
    0x00, 0x08, (uint8_t) 0x00, 0x96,
    0x08, 0x08, (uint8_t) 0x00, 0x97,    
};

const uint8_t walkerSprite2[] PROGMEM = {
    0x04, 0x10, 0x10,
    0x00, 0x00, (uint8_t) 0x00, 0x88,
    0x08, 0x00, (uint8_t) 0x00, 0x89,
    0x00, 0x08, (uint8_t) 0x00, 0x98,
    0x08, 0x08, (uint8_t) 0x00, 0x99,    
};

const uint8_t* const spriteAddresses[SPRITE_TYPE_COUNT] PROGMEM = {
    playerSprite1,
    playerSprite2,
    fireballSprite1,
    fireballSprite2,
    circleSprite1,
    walkerSprite1,
    walkerSprite2,
};



void spriteDraw(int16_t x, int16_t y, SpriteType sprite, uint8_t flags) {
    const uint8_t* data = (const uint8_t*) pgm_read_word(spriteAddresses + (uint8_t) sprite);
    
    uint8_t len = pgm_read_byte(data++);
    uint8_t w = pgm_read_byte(data++);
    uint8_t h = pgm_read_byte(data++);

    for(uint8_t i = 0; i != len; ++i) {
        uint8_t xofs = pgm_read_byte(data++);
        uint8_t yofs = pgm_read_byte(data++);
        uint8_t attr = pgm_read_byte(data++);
        uint8_t tile = pgm_read_byte(data++);
        uint8_t xorattr = attr ^ flags;

        int16_t tx = x + ((flags & SPRITE_ATTR_HFLIP) != 0 ? w - xofs - 8 : xofs);
        int16_t ty = y + ((flags & SPRITE_ATTR_VFLIP) != 0 ? h - yofs - 8 : yofs);
        bool hflip = (xorattr & (uint8_t) SPRITE_ATTR_HFLIP) != 0;
        bool vflip = (xorattr & (uint8_t) SPRITE_ATTR_VFLIP) != 0;

        arduboy.drawTile(tx, ty, spritesBitmap, tile, (flags & SPRITE_FLAG_COLOR_INVERT) == 0 ? 0 : 1, hflip, vflip);
        arduboy.drawTile(tx, ty, spritesBitmap + 2048, tile, (flags & SPRITE_FLAG_COLOR_INVERT) == 0 ? 1 : 0, hflip, vflip);        
    }
}
