#include <stdlib.h>
#include <string.h>
#include "entity.h"
#include "sprite.h"
#include "map.h"
#include "hitbox.h"

Entity ents[ENT_COUNT];

enum {
    ENT_MAXIMUM_SLOPE = 4,
    ENT_SCREEN_BORDER_X = 64,
    ENT_SCREEN_BORDER_Y = 16,
};

void entityInitSystem() {
    memset(ents, 0, sizeof(ents));
}

uint8_t entityAdd(int16_t x, int16_t y, uint8_t offset, uint8_t count) {
    uint8_t index = offset;    
    while(count != 0) {
        Entity* ent = &ents[index];
        if(ent->controlFlags == 0) {
            memset(ent, 0, sizeof(Entity));
            ent->controlFlags = ENT_CTRL_FLAG_ACTIVE;
            ent->x = x;
            ent->y = y;
            return index;
        }

        ++index;
        --count;
    }

    return 0xFF;
}

void entityRemove(uint8_t entityIndex) {
    ents[entityIndex].controlFlags = 0;
}

static bool detectFloor(Entity* ent, int8_t hx, int8_t hy, int8_t hw, int8_t hh) {
    int16_t checkPixelX = ent->x / 16 + hx;
    int16_t checkPixelY = ent->y / 16 + hy;

    return mapGetPixelObs(checkPixelX, checkPixelY + hh)
        || mapGetPixelObs(checkPixelX + hw / 2, checkPixelY + hh)
        || mapGetPixelObs(checkPixelX + hw - 1, checkPixelY + hh);
}

bool entityDetectFloor(uint8_t entityIndex) {
    Entity* ent = &ents[entityIndex];
    int8_t hx, hy, hw, hh;
    hitboxGetData((HitboxType) ent->hitbox, &hx, &hy, &hw, &hh);
    return detectFloor(ent, hx, hy, hw, hh);
}

void entityUpdate(uint8_t entityIndex) {
    Entity* ent = &ents[entityIndex];
    if((ent->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
        if((ent->controlFlags & ENT_CTRL_FLAG_IGNORE_OBS) != 0) {
            ent->x += ent->xspd;
            ent->y += ent->yspd;
        } else {
            int8_t hx, hy, hw, hh;
            hitboxGetData((HitboxType) ent->hitbox, &hx, &hy, &hw, &hh);

            int16_t checkPixelX = (ent->x + ent->xspd) / 16 + hx;
            int16_t checkPixelY = ent->y / 16 + hy;
            int8_t maxSlope = (ent->controlFlags & ENT_CTRL_FLAG_IGNORE_SLOPES) != 0 ? 0 : ENT_MAXIMUM_SLOPE;

            ent->status &= ~(ENT_STATUS_HIT_OBS_X | ENT_STATUS_HIT_OBS_Y);
            if((ent->xspd < 0
                && !mapGetPixelObs(checkPixelX - 1, checkPixelY)
                && !mapGetPixelObs(checkPixelX - 1, checkPixelY + hh / 2)
                && !mapGetPixelObs(checkPixelX - 1, checkPixelY - (maxSlope * -ent->xspd / 16) + hh - 1 - maxSlope))
            || (ent->xspd > 0
                && !mapGetPixelObs(checkPixelX + hw, checkPixelY)
                && !mapGetPixelObs(checkPixelX + hw, checkPixelY + hh / 2)
                && !mapGetPixelObs(checkPixelX + hw, checkPixelY - (maxSlope * ent->xspd / 16) + hh - 1 - maxSlope))) {
                ent->x += ent->xspd;
                if(ent->yspd == 0 && (ent->controlFlags & ENT_CTRL_FLAG_IGNORE_SLOPES) == 0 && !detectFloor(ent, hx, hy, hw, hh)) {
                    int16_t descent = abs(ent->xspd) * ENT_MAXIMUM_SLOPE;
                    if(descent >= ENT_MAXIMUM_SLOPE * 16) {
                        descent = ENT_MAXIMUM_SLOPE * 16;
                    }
                    ent->y += descent;
                    if(!detectFloor(ent, hx, hy, hw, hh)) {
                        ent->y -= descent;
                    }
                }
            } else if(ent->xspd != 0) {
                //ent->xspd = 0;
                ent->status |= ENT_STATUS_HIT_OBS_X;
            }

            checkPixelX = ent->x / 16 + hx;
            checkPixelY = (ent->y + ent->yspd) / 16 + hy;

            if((ent->yspd < 0
                && !mapGetPixelObs(checkPixelX, checkPixelY - 1)
                && !mapGetPixelObs(checkPixelX + hw / 2, checkPixelY - 1)
                && !mapGetPixelObs(checkPixelX + hw - 1, checkPixelY - 1))
            || (ent->yspd > 0
                && !mapGetPixelObs(checkPixelX, checkPixelY + hh - 1)
                && !mapGetPixelObs(checkPixelX + hw / 2, checkPixelY + hh - 1)
                && !mapGetPixelObs(checkPixelX + hw - 1, checkPixelY + hh - 1))) {
                ent->y += ent->yspd;
            } else if(ent->yspd != 0) {
                ent->yspd = 0;
                ent->status |= ENT_STATUS_HIT_OBS_Y;
            }

            if((ent->controlFlags & ENT_CTRL_FLAG_IGNORE_SLOPES) == 0) {
                uint8_t preventClipOffset = 0;
                while(detectFloor(ent, hx, hy, hw, hh) && ent->y > -256) {
                    ent->y -= 16;
                    preventClipOffset++;
                    if(preventClipOffset == 0) preventClipOffset = 255;
                }
                ent->status &= ~ENT_STATUS_UPHILL;
                if(preventClipOffset > 0) {
                    ent->y += 16;
                    if(preventClipOffset > 1) {
                        ent->status |= ENT_STATUS_UPHILL;
                    }
                }
            }
        }
    }
}

void entityDraw(uint8_t entityIndex) {
    Entity* ent = &ents[entityIndex];
    if((ent->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0
    && (ent->drawFlags & ENT_DRAW_FLAG_HIDDEN) == 0) {
        uint8_t entityFlags = ent->drawFlags;
        uint8_t spriteFlags = 0;
        if((entityFlags & ENT_DRAW_FLAG_HFLIP) != 0) {
            spriteFlags |= SPRITE_FLAG_HFLIP;
        }
        if((entityFlags & ENT_DRAW_FLAG_VFLIP) != 0) {
            spriteFlags |= SPRITE_FLAG_VFLIP;
        }
        if((entityFlags & ENT_DRAW_FLAG_FLASH) != 0) {
            spriteFlags |= SPRITE_FLAG_COLOR_INVERT;
        }        
        spriteDraw(ent->x / 16 - mapCameraX, ent->y / 16 - mapCameraY, (SpriteType) ent->sprite, spriteFlags);
    }
}

bool entityCollide(uint8_t entityIndexA, int8_t borderA, uint8_t entityIndexB, int8_t borderB) {
    Entity* entA = &ents[entityIndexA];
    Entity* entB = &ents[entityIndexB];
    if((entA->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0
    && (entB->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
        return hitboxCollide(entA->x / 16, entA->y / 16, (HitboxType) entA->hitbox, borderA, entB->x / 16, entB->y / 16, (HitboxType) entB->hitbox, borderB);
    }    
    return false;
}
