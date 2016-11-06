#include <string.h>

#include "frogboy.h"
#include "player.h"
#include "sprite.h"
#include "bullet.h"
#include "hitbox.h"
#include "particle.h"
#include "sprites_bitmap.h"

enum {
    PLAYER_MAX_HP_START = 3,

    PLAYER_RUN_XACCEL = 6,
    PLAYER_RUN_MAX_XSPEED = 24,

    PLAYER_FRICTION_MULTIPLIER = 14,
    PLAYER_FRICTION_DIVIDER = 16,

    PLAYER_JUMP_MIN_DURATION = 8,
    PLAYER_JUMP_MAX_DURATION = 14,
    PLAYER_JUMP_YACCEL = 3,
    PLAYER_JUMP_MAX_YSPEED = 32,
    PLAYER_JUMP_INPUT_BUFFER_TIME = 10,

    PLAYER_FALL_YACCEL = 2,
    PLAYER_FALL_MAX_YSPEED = 32,

    PLAYER_SHOOT_COOLDOWN = 7,
    PLAYER_SHOOT_MAX_SHOTS = 2,
    PLAYER_SHOOT_INPUT_BUFFER_TIME = 16,
};

Player player;

void playerAdd(int16_t x, int16_t y) {
    uint8_t entityIndex = entityAdd(x, y, ENT_OFFSET_PLAYER, ENT_COUNT_PLAYER);

    if(entityIndex != 0xFF) {
        memset(&player, 0, sizeof(Player));

        Entity* ent = &ents[entityIndex];
        ent->sprite = (uint8_t) SPRITE_TYPE_PLAYER_1;
        ent->hitbox = HITBOX_TYPE_HUMAN_16x16;

        player.hp = 1;
        player.maxHP = PLAYER_MAX_HP_START;
        player.shotCount = 0;
        player.maxShotCount = PLAYER_SHOOT_MAX_SHOTS;
    }
}

void playerUpdate() {
    bool moved = false;
    Entity* ent = &ents[ENT_OFFSET_PLAYER];

    entityUpdate(ENT_OFFSET_PLAYER);

    bool left = frogboy::isPressed(frogboy::BUTTON_LEFT);
    bool right = frogboy::isPressed(frogboy::BUTTON_RIGHT);
    if(left && right) {
        left = right = false;
    }
    
    if(left) {
        ent->xspd -= PLAYER_RUN_XACCEL;
        if(ent->xspd < -PLAYER_RUN_MAX_XSPEED) {
            ent->xspd = -PLAYER_RUN_MAX_XSPEED;
        }        
        player.dir = false;
        moved = true;
    }
    else if(right) {
        ent->xspd += PLAYER_RUN_XACCEL;
        if(ent->xspd > PLAYER_RUN_MAX_XSPEED) {
            ent->xspd = PLAYER_RUN_MAX_XSPEED;
        }
        player.dir = true;
        moved = true;
    } else {
        ent->xspd = ent->xspd * PLAYER_FRICTION_MULTIPLIER / PLAYER_FRICTION_DIVIDER;
    }

    if(player.shootTimer > 0) {
        player.shootTimer--;
    }

    if(frogboy::isPressed(frogboy::BUTTON_SHOOT)) {
        if(!player.shootPressed) {
            player.bufferShootTimer = PLAYER_SHOOT_INPUT_BUFFER_TIME;
            player.shootPressed = true;
        }
    } else {
        player.shootPressed = false;
        if(player.bufferShootTimer > 0) {
            player.bufferShootTimer--;
        }
    }

    if(player.bufferShootTimer > 0) {
        if(player.shootTimer == 0 && player.shotCount < player.maxShotCount) {
            bulletAdd(ent->x + (player.dir ? 8 * 16 : -8 * 16), ent->y, player.dir, BULLET_TYPE_FIREBALL);
            player.bufferShootTimer = 0;
            player.shootTimer = PLAYER_SHOOT_COOLDOWN;
            player.shotCount++;
        }
    }

    if(frogboy::isPressed(frogboy::BUTTON_JUMP)) {
        if(!player.jumpPressed) {
            player.bufferJumpTimer = PLAYER_JUMP_INPUT_BUFFER_TIME;
            player.jumpPressed = true;
        }
    } else {
        player.jumpPressed = false;
        if(player.bufferJumpTimer > 0) {
            player.bufferJumpTimer--;
        }
        if(player.jumpTimer <= PLAYER_JUMP_MAX_DURATION - PLAYER_JUMP_MIN_DURATION) {
            player.jumpTimer = 0;
        }        
    }

    if(player.bufferJumpTimer > 0) {
        if(player.landed) {
            player.bufferJumpTimer = 0;
            player.landed = false;
            player.jumpTimer = PLAYER_JUMP_MAX_DURATION;
        }
    }

    if(player.jumpTimer > 0) {
        player.landed = false;
        player.jumpTimer--;
        ent->yspd -= PLAYER_JUMP_YACCEL;
        if(ent->yspd < -PLAYER_JUMP_MAX_YSPEED) {
            ent->yspd = -PLAYER_JUMP_MAX_YSPEED;
        }
    }

    if(player.jumpTimer == 0) {
        if(!entityDetectFloor(ENT_OFFSET_PLAYER)) {
            ent->yspd += PLAYER_FALL_YACCEL;
            if(ent->yspd > PLAYER_FALL_MAX_YSPEED) {
                ent->yspd = PLAYER_FALL_MAX_YSPEED;
            }
            player.landed = false;
            player.falling = true;
        } else {
            if(!player.landed) {
                player.landed = true;
                ent->yspd = 0;

                if(player.falling) {
                    player.falling = false;
                    particleStarAdd(ent->x + 10 * 16, ent->y + 4 * 16);
                }
            }
        }
    }
    
    if(moved) {
        player.timer++;
        if(player.timer >= 16) {
            player.timer = 0;    
        }
    } else {
        player.timer = 0;
    }

    if(player.dir) {
        ent->drawFlags |= ENT_DRAW_FLAG_HFLIP;
    } else {
        ent->drawFlags &= ~ENT_DRAW_FLAG_HFLIP;
    }

    if(player.jumpTimer != 0 || player.falling) {
        ent->sprite = SPRITE_TYPE_PLAYER_2;
    } else {
        ent->sprite = player.timer < 1 || player.timer >= 9 ? SPRITE_TYPE_PLAYER_1 : SPRITE_TYPE_PLAYER_2;
    }
}

void playerDraw() {
    entityDraw(ENT_OFFSET_PLAYER);
}

void playerDrawHUD() {
    uint8_t h;
    for(h = 0; h < player.maxHP; ++h) {
        frogboy::drawTile(2 + h * 8, 3, spritesBitmap, 0x40, 0, false, false);
    }
    for(h = 0; h < player.hp; ++h) {
        frogboy::drawTile(2 + h * 8, 2, spritesBitmap, 0x40, 1, false, false);
    }
    for(; h < player.maxHP; ++h) {
        frogboy::drawTile(2 + h * 8, 2, spritesBitmap, 0x41, 1, false, false);
    }    
}
