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

const uint8_t collisionCategoryCollidesWith[COLLISION_CATEGORY_COUNT] FROGBOY_ROM_DATA = {
    COLLISION_MASK_BLOCK,
    COLLISION_MASK_CRITTER | COLLISION_MASK_BLOCK,
    0,
};

void entityInitSystem() {
    memset(ents, 0, sizeof(ents));
}

Entity* entityAdd(int16_t x, int16_t y, uint8_t offset, uint8_t count) {
    uint8_t index = offset;    
    Entity* ent = &ents[index];
    while(count != 0) {
        if(ent->controlFlags == 0) {
            memset(ent, 0, sizeof(Entity));
            ent->controlFlags = ENT_CTRL_FLAG_ACTIVE;
            ent->x = x;
            ent->y = y;
            return ent;
        }

        ++index;
        ++ent;
        --count;
    }

    return nullptr;
}

void entityRemove(Entity* ent) {
    ent->controlFlags = 0;
}

static bool entGetEntPixelObs(Entity* ent, int16_t x, int16_t y) {
    Entity* wallEnt = &ents[ENT_OFFSET_CRITTER];
    Entity* wallEntEnd = wallEnt + ENT_COUNT_CRITTER;
    uint8_t collidesWith = frogboy::readRom(&collisionCategoryCollidesWith[ent->collisionCategory]);
    for(; wallEnt != wallEntEnd; ++wallEnt) {
        if(wallEnt != ent
        && (wallEnt->controlFlags & ENT_CTRL_FLAG_ACTIVE) == ENT_CTRL_FLAG_ACTIVE
        && (collidesWith & (1 << wallEnt->collisionCategory)) != 0) {
            if(hitboxCollide((wallEnt->x + wallEnt->xspd) / 16, wallEnt->y / 16, (HitboxType) wallEnt->hitbox, 0, x, y, HITBOX_TYPE_1x1, 0)) {
                return true;
            }
        }
    }
    return false;
}

static bool entGetAnyPixelObs(Entity* ent, int16_t x, int16_t y) {    
    return entGetEntPixelObs(ent, x, y) || mapGetPixelObs(x, y);
}

static bool detectFloor(Entity* ent, int8_t hx, int8_t hy, int8_t hw, int8_t hh, bool ignoreEnts) {
    int16_t checkPixelX = ent->x / 16 + hx;
    int16_t checkPixelY = ent->y / 16 + hy;

    if(ignoreEnts) {
        return mapGetPixelObs(checkPixelX, checkPixelY + hh)
            || mapGetPixelObs(checkPixelX + hw / 2, checkPixelY + hh)
            || mapGetPixelObs(checkPixelX + hw - 1, checkPixelY + hh);
    } else {
        return entGetAnyPixelObs(ent, checkPixelX, checkPixelY + hh)
            || entGetAnyPixelObs(ent, checkPixelX + hw / 2, checkPixelY + hh)
            || entGetAnyPixelObs(ent, checkPixelX + hw - 1, checkPixelY + hh);
    }
}

bool entityDetectFloor(Entity* ent) {
    int8_t hx, hy, hw, hh;
    hitboxGetData((HitboxType) ent->hitbox, &hx, &hy, &hw, &hh);
    return detectFloor(ent, hx, hy, hw, hh, false);
}

void entityUpdate(Entity* ent) {
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

            ent->status &= ~(ENT_STATUS_HIT_OBS_X | ENT_STATUS_HIT_OBS_Y | ENT_STATUS_UPHILL | ENT_STATUS_DOWNHILL);
            if((ent->xspd < 0
                && !entGetAnyPixelObs(ent, checkPixelX - 1, checkPixelY)
                && !entGetAnyPixelObs(ent, checkPixelX - 1, checkPixelY + hh / 2)
                && !entGetAnyPixelObs(ent, checkPixelX - 1, checkPixelY - (maxSlope * -ent->xspd / 16) + hh - 1 - maxSlope)
                && !entGetEntPixelObs(ent, checkPixelX - 1, checkPixelY + hh - 1))
            || (ent->xspd > 0
                && !entGetAnyPixelObs(ent, checkPixelX + hw, checkPixelY)
                && !entGetAnyPixelObs(ent, checkPixelX + hw, checkPixelY + hh / 2)
                && !entGetAnyPixelObs(ent, checkPixelX + hw, checkPixelY - (maxSlope * ent->xspd / 16) + hh - 1 - maxSlope)
                && !entGetEntPixelObs(ent, checkPixelX + hw, checkPixelY + hh - 1))) {
                ent->x += ent->xspd;
                if(ent->yspd == 0 && (ent->controlFlags & ENT_CTRL_FLAG_IGNORE_SLOPES) == 0 && !detectFloor(ent, hx, hy, hw, hh, true)) {
                    int16_t descent = abs(ent->xspd) * ENT_MAXIMUM_SLOPE;
                    if(descent >= ENT_MAXIMUM_SLOPE * 16) {
                        descent = ENT_MAXIMUM_SLOPE * 16;
                    } 
                    ent->y += descent;
                    if(detectFloor(ent, hx, hy, hw, hh, true)) {
                        ent->status |= ENT_STATUS_DOWNHILL;
                    } else {
                        ent->y -= descent;
                    }
                }
            } else if(ent->xspd != 0) {
                ent->status |= ENT_STATUS_HIT_OBS_X;
            }

            checkPixelX = ent->x / 16 + hx;
            checkPixelY = (ent->y + ent->yspd) / 16 + hy;

            if((ent->yspd < 0
                && !entGetAnyPixelObs(ent, checkPixelX, checkPixelY - 1)
                && !entGetAnyPixelObs(ent, checkPixelX + hw / 2, checkPixelY - 1)
                && !entGetAnyPixelObs(ent, checkPixelX + hw - 1, checkPixelY - 1))
            || (ent->yspd > 0
                && !entGetAnyPixelObs(ent, checkPixelX, checkPixelY + hh - 1)
                && !entGetAnyPixelObs(ent, checkPixelX + hw / 2, checkPixelY + hh - 1)
                && !entGetAnyPixelObs(ent, checkPixelX + hw - 1, checkPixelY + hh - 1))) {
                ent->y += ent->yspd;
            } else if(ent->yspd != 0) {
                ent->yspd = 0;
                ent->status |= ENT_STATUS_HIT_OBS_Y;
            }

            if((ent->controlFlags & ENT_CTRL_FLAG_IGNORE_SLOPES) == 0) {
                uint8_t preventClipOffset = 0;
                while(detectFloor(ent, hx, hy, hw, hh, true) && ent->y > -256) {
                    ent->y -= 16;
                    preventClipOffset++;
                    if(preventClipOffset == 0) preventClipOffset = 255;
                }
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

void entityDraw(Entity* ent) {
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

bool entityCollide(Entity* entA, int8_t borderA, Entity* entB, int8_t borderB) {
    if((entA->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0
    && (entB->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
        return hitboxCollide(entA->x / 16, entA->y / 16, (HitboxType) entA->hitbox, borderA, entB->x / 16, entB->y / 16, (HitboxType) entB->hitbox, borderB);
    }    
    return false;
}