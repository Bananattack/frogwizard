#include <stdlib.h>
#include <string.h>
#include "entity.h"
#include "sprite.h"
#include "text.h"
#include "map.h"
#include "camera.h"
#include "hitbox.h"
#include "game.h"

namespace {    
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
}

Entity Entity::data[ENT_COUNT];

void Entity::initSystem() {
    memset(data, 0, sizeof(data));
}

Entity* Entity::add(int16_t x, int16_t y, uint8_t offset, uint8_t count) {
    uint8_t index = offset;    
    Entity* ent = &data[index];
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

void Entity::remove() {
    controlFlags = 0;
}

bool Entity::getEntPixelObs(int16_t x, int16_t y) {
    Entity* wallEnt = &data[ENT_OFFSET_CRITTER];
    Entity* wallEntEnd = wallEnt + ENT_COUNT_CRITTER;
    uint8_t collidesWith = frogboy::readRom(&collisionCategoryCollidesWith[collisionCategory]);
    for(; wallEnt != wallEntEnd; ++wallEnt) {
        if(wallEnt != this
        && (wallEnt->controlFlags & ENT_CTRL_FLAG_ACTIVE) == ENT_CTRL_FLAG_ACTIVE
        && (collidesWith & (1 << wallEnt->collisionCategory)) != 0) {
            if(hitbox::collide((wallEnt->x + wallEnt->xspd) / 16, wallEnt->y / 16, (HitboxType) wallEnt->hitbox, 0, x, y, HITBOX_TYPE_1x1, 0)) {
                return true;
            }
        }
    }
    return false;
}

bool Entity::getAnyPixelObs(int16_t x, int16_t y) {    
    return getEntPixelObs(x, y) || map.getPixelObs(x, y);
}

bool Entity::detectFloor(int8_t hx, int8_t hy, int8_t hw, int8_t hh, bool ignoreEnts) {
    int16_t checkPixelX = x / 16 + hx;
    int16_t checkPixelY = y / 16 + hy;

    if(ignoreEnts) {
        return map.getPixelObs(checkPixelX, checkPixelY + hh)
            || map.getPixelObs(checkPixelX + hw / 2, checkPixelY + hh)
            || map.getPixelObs(checkPixelX + hw - 1, checkPixelY + hh);
    } else {
        return getAnyPixelObs(checkPixelX, checkPixelY + hh)
            || getAnyPixelObs(checkPixelX + hw / 2, checkPixelY + hh)
            || getAnyPixelObs(checkPixelX + hw - 1, checkPixelY + hh);
    }
}

bool Entity::detectFloor() {
    int8_t hx, hy, hw, hh;
    hitbox::read(static_cast<HitboxType>(hitbox), &hx, &hy, &hw, &hh);
    return detectFloor(hx, hy, hw, hh, false);
}

void Entity::update() {
    if((controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
        if((controlFlags & ENT_CTRL_FLAG_IGNORE_OBS) != 0) {
            x += xspd;
            y += yspd;
        } else {
            int8_t hx, hy, hw, hh;
            hitbox::read(static_cast<HitboxType>(hitbox), &hx, &hy, &hw, &hh);

            int16_t checkPixelX = (x + xspd) / 16 + hx;
            int16_t checkPixelY = y / 16 + hy;
            int8_t maxSlope = (controlFlags & ENT_CTRL_FLAG_IGNORE_SLOPES) != 0 ? 0 : ENT_MAXIMUM_SLOPE;

            status &= ~(ENT_STATUS_HIT_OBS_X | ENT_STATUS_HIT_OBS_Y | ENT_STATUS_UPHILL | ENT_STATUS_DOWNHILL);
            if((xspd < 0
                && !getAnyPixelObs(checkPixelX - 1, checkPixelY)
                && !getAnyPixelObs(checkPixelX - 1, checkPixelY + hh / 2)
                && !getAnyPixelObs(checkPixelX - 1, checkPixelY - (maxSlope * -xspd / 16) + hh - 1 - maxSlope)
                && !getEntPixelObs(checkPixelX - 1, checkPixelY + hh - 1))
            || (xspd > 0
                && !getAnyPixelObs(checkPixelX + hw, checkPixelY)
                && !getAnyPixelObs(checkPixelX + hw, checkPixelY + hh / 2)
                && !getAnyPixelObs(checkPixelX + hw, checkPixelY - (maxSlope * xspd / 16) + hh - 1 - maxSlope)
                && !getEntPixelObs(checkPixelX + hw, checkPixelY + hh - 1))) {
                x += xspd;
                if(yspd == 0 && (controlFlags & ENT_CTRL_FLAG_IGNORE_SLOPES) == 0 && !detectFloor(hx, hy, hw, hh, true)) {
                    int16_t descent = abs(xspd) * ENT_MAXIMUM_SLOPE;
                    if(descent >= ENT_MAXIMUM_SLOPE * 16) {
                        descent = ENT_MAXIMUM_SLOPE * 16;
                    } 
                    y += descent;
                    if(detectFloor(hx, hy, hw, hh, true)) {
                        status |= ENT_STATUS_DOWNHILL;
                    } else {
                        y -= descent;
                    }
                }
            } else if(xspd != 0) {
                status |= ENT_STATUS_HIT_OBS_X;
            }

            checkPixelX = x / 16 + hx;
            checkPixelY = (y + yspd) / 16 + hy;

            if((yspd < 0
                && !getAnyPixelObs(checkPixelX, checkPixelY - 1)
                && !getAnyPixelObs(checkPixelX + hw / 2, checkPixelY - 1)
                && !getAnyPixelObs(checkPixelX + hw - 1, checkPixelY - 1))
            || (yspd > 0
                && !getAnyPixelObs(checkPixelX, checkPixelY + hh - 1)
                && !getAnyPixelObs(checkPixelX + hw / 2, checkPixelY + hh - 1)
                && !getAnyPixelObs(checkPixelX + hw - 1, checkPixelY + hh - 1))) {
                y += yspd;
            } else if(yspd != 0) {
                yspd = 0;
                status |= ENT_STATUS_HIT_OBS_Y;
            }

            if((controlFlags & ENT_CTRL_FLAG_IGNORE_SLOPES) == 0) {
                uint8_t preventClipOffset = 0;
                while(detectFloor(hx, hy, hw, hh, true) && y > -256) {
                    y -= 16;
                    preventClipOffset++;
                    if(preventClipOffset == 0) preventClipOffset = 255;
                }
                if(preventClipOffset > 0) {
                    y += 16;
                    if(preventClipOffset > 1) {
                        status |= ENT_STATUS_UPHILL;
                    }
                }
            }
        }
    }
}

void Entity::draw() {
    uint8_t entityFlags = drawFlags;
    if((controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0
    && (entityFlags & ENT_DRAW_FLAG_HIDDEN) == 0) {
        int16_t drawX = x / 16 - camera.x;
        int16_t drawY = y / 16 - camera.y;

        if((entityFlags & ENT_DRAW_FLAG_TEXT) != 0) {
            if(gameMode != GAME_MODE_PAUSE) {
                text::print(drawX, drawY, static_cast<TextType>(sprite), 1);
            }
        } else {
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
            sprite::draw(drawX, drawY, static_cast<SpriteType>(sprite), spriteFlags);
        }
    }
}

bool Entity::collide(int8_t thisBorder, Entity* other, int8_t otherBorder) {
    if((controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0
    && (other->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
        return hitbox::collide(x / 16, y / 16, static_cast<HitboxType>(hitbox), thisBorder, other->x / 16, other->y / 16, static_cast<HitboxType>(other->hitbox), otherBorder);
    }    
    return false;
}