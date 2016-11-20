#include "frogboy.h"
#include "sprite.h"
#include "sprites_bitmap.h"

namespace {
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

    const uint8_t playerSprite1[] FROGBOY_ROM_DATA = {
        0x04, 0x10, 0x10,
        0x00, 0x00, 0, 0x66,
        0x08, 0x00, 0, 0x67,
        0x00, 0x08, 0, 0x76,
        0x08, 0x08, 0, 0x77,    
    };

    const uint8_t playerSprite2[] FROGBOY_ROM_DATA = {
        0x04, 0x10, 0x10,
        0x00, 0x00, 0, 0x68,
        0x08, 0x00, 0, 0x69,
        0x00, 0x08, 0, 0x78,
        0x08, 0x08, 0, 0x79,    
    };

    const uint8_t fireballSprite1[] FROGBOY_ROM_DATA = {
        0x04, 0x10, 0x10,
        0x00, 0x00, 0, 0x80,
        0x08, 0x00, 0, 0x81,
        0x00, 0x08, 0, 0x90,
        0x08, 0x08, 0, 0x91,    
    };

    const uint8_t fireballSprite2[] FROGBOY_ROM_DATA = {
        0x04, 0x10, 0x10,
        0x00, 0x00, 0, 0x82,
        0x08, 0x00, 0, 0x83,
        0x00, 0x08, 0, 0x92,
        0x08, 0x08, 0, 0x93,    
    };

    const uint8_t circleSprite1[] FROGBOY_ROM_DATA = {
        0x04, 0x10, 0x10,
        0x00, 0x00, 0, 0x84,
        0x08, 0x00, SPRITE_FLAG_HFLIP, 0x84,
        0x00, 0x08, SPRITE_FLAG_VFLIP, 0x84,
        0x08, 0x08, SPRITE_FLAG_HFLIP | SPRITE_FLAG_VFLIP, 0x84,
    };

    const uint8_t walkerSprite1[] FROGBOY_ROM_DATA = {
        0x04, 0x10, 0x10,
        0x00, 0x00, 0, 0x86,
        0x08, 0x00, 0, 0x87,
        0x00, 0x08, 0, 0x96,
        0x08, 0x08, 0, 0x97,    
    };

    const uint8_t walkerSprite2[] FROGBOY_ROM_DATA = {
        0x04, 0x10, 0x10,
        0x00, 0x00, 0, 0x88,
        0x08, 0x00, 0, 0x89,
        0x00, 0x08, 0, 0x98,
        0x08, 0x08, 0, 0x99,    
    };

    const uint8_t doorSprite1[] FROGBOY_ROM_DATA = {
        0x04, 0x10, 0x10,
        0x00, 0x00, 0, 0x4B,
        0x08, 0x00, SPRITE_FLAG_HFLIP, 0x4B,
        0x00, 0x08, 0, 0x5B,
        0x08, 0x08, SPRITE_FLAG_HFLIP, 0x5B,
    };

    const uint8_t blockSprite1[] FROGBOY_ROM_DATA = {
        0x04, 0x10, 0x10,
        0x00, 0x00, 0, 0x6A,
        0x08, 0x00, SPRITE_FLAG_HFLIP, 0x6A,
        0x00, 0x08, SPRITE_FLAG_VFLIP, 0x6A,
        0x08, 0x08, SPRITE_FLAG_HFLIP | SPRITE_FLAG_VFLIP, 0x6A,
    };

    const uint8_t eggSprite1[] FROGBOY_ROM_DATA = {
        0x04, 0x10, 0x10,
        0x00, 0x00, 0, 0x8A,
        0x08, 0x00, SPRITE_FLAG_HFLIP, 0x8A,
        0x00, 0x08, 0, 0x9A,
        0x08, 0x08, SPRITE_FLAG_HFLIP, 0x9A,
    };

    const uint8_t* const spritePointers[SPRITE_TYPE_COUNT] FROGBOY_ROM_DATA = {
        playerSprite1,
        playerSprite2,
        fireballSprite1,
        fireballSprite2,
        circleSprite1,
        walkerSprite1,
        walkerSprite2,
        doorSprite1,
        blockSprite1,
        eggSprite1,
    };
}

namespace sprite {
    void draw(int16_t x, int16_t y, SpriteType sprite, uint8_t flags) {
        if(sprite < SPRITE_TYPE_COUNT) {
            const uint8_t* data = frogboy::readRom<const uint8_t*>(spritePointers + sprite);
    
            uint8_t len = frogboy::readRom<uint8_t>(data++);
            uint8_t w = frogboy::readRom<uint8_t>(data++);
            uint8_t h = frogboy::readRom<uint8_t>(data++);

            for(uint8_t i = 0; i != len; ++i) {
                uint8_t xofs = frogboy::readRom<uint8_t>(data++);
                uint8_t yofs = frogboy::readRom<uint8_t>(data++);
                uint8_t attr = frogboy::readRom<uint8_t>(data++);
                uint8_t tile = frogboy::readRom<uint8_t>(data++);
                uint8_t xorattr = attr ^ flags;

                int16_t tx = x + ((flags & SPRITE_FLAG_HFLIP) != 0 ? w - xofs - 8 : xofs);
                int16_t ty = y + ((flags & SPRITE_FLAG_VFLIP) != 0 ? h - yofs - 8 : yofs);
                bool hflip = (xorattr & SPRITE_FLAG_HFLIP) != 0;
                bool vflip = (xorattr & SPRITE_FLAG_VFLIP) != 0;
                bool invert = (xorattr & SPRITE_FLAG_COLOR_INVERT) != 0;

                frogboy::drawTile(tx, ty, spritesBitmap, tile, invert ? 1 : 0, hflip, vflip);
                frogboy::drawTile(tx, ty, spritesBitmap + 2048, tile, invert ? 0 : 1, hflip, vflip);        
            }
        }
    }
}
