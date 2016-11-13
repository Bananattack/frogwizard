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
    PLAYER_RUN_MAX_XSPEED = 20,

    PLAYER_FRICTION_MULTIPLIER = 14,
    PLAYER_FRICTION_DIVIDER = 16,

    PLAYER_JUMP_MIN_DURATION = 4,
    PLAYER_JUMP_MAX_DURATION = 9,
    PLAYER_JUMP_YACCEL = 28,
    PLAYER_JUMP_MAX_YSPEED = 28,
    PLAYER_JUMP_INPUT_BUFFER_TIME = 10,

    PLAYER_FALL_YACCEL = 2,
    PLAYER_FALL_MAX_YSPEED = 32,

    PLAYER_SHOOT_COOLDOWN = 7,
    PLAYER_SHOOT_MAX_SHOTS = 2,
    PLAYER_SHOOT_INPUT_BUFFER_TIME = 16,

    PLAYER_HURT_DURATION = 60,
    PLAYER_HURT_BLINK_INTERVAL = 4,
    PLAYER_HURT_STOP_CONTROL_DURATION = 16,
};

PlayerStatus playerStatus;
Player player;

void playerInitSystem() {
    memset(&playerStatus, 0, sizeof(PlayerStatus));
    playerStatus.hp = PLAYER_MAX_HP_START;
    playerStatus.maxHP = PLAYER_MAX_HP_START;
    playerStatus.maxShotCount = PLAYER_SHOOT_MAX_SHOTS;
    playerStatus.nextMap = 0xFF;
    playerStatus.dir = true;
    playerStatus.usedDoor = false;
}

void playerAdd(int16_t x, int16_t y) {
    uint8_t entityIndex = entityAdd(x, y, ENT_OFFSET_PLAYER, ENT_COUNT_PLAYER);

    if(entityIndex != 0xFF) {
        memset(&player, 0, sizeof(Player));

        Entity* ent = &ents[entityIndex];
        ent->sprite = SPRITE_TYPE_PLAYER_1;
        ent->hitbox = HITBOX_TYPE_HUMAN_16x16;
        if(playerStatus.dir) {
            ent->drawFlags |= ENT_DRAW_FLAG_HFLIP;
        }
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

    if(playerStatus.usedDoor && !frogboy::isPressed(frogboy::BUTTON_UP)) {
        playerStatus.usedDoor = false;
    }

    ent->drawFlags &= ~ENT_DRAW_FLAG_HIDDEN;
    if(player.hurtTimer > 0) {
        player.hurtTimer--;
        if(player.hurtTimer % (PLAYER_HURT_BLINK_INTERVAL * 2) < PLAYER_HURT_BLINK_INTERVAL) {
            ent->drawFlags |= ENT_DRAW_FLAG_HIDDEN;
        }
        if(player.hurtTimer > PLAYER_HURT_DURATION - PLAYER_HURT_STOP_CONTROL_DURATION)  {
            left = right = false;
        }
    }
    
    if(left) {
        ent->xspd -= PLAYER_RUN_XACCEL;
        if(ent->xspd < -PLAYER_RUN_MAX_XSPEED) {
            ent->xspd = -PLAYER_RUN_MAX_XSPEED;
        }        
        playerStatus.dir = false;
        moved = true;
    }
    else if(right) {
        ent->xspd += PLAYER_RUN_XACCEL;
        if(ent->xspd > PLAYER_RUN_MAX_XSPEED) {
            ent->xspd = PLAYER_RUN_MAX_XSPEED;
        }
        playerStatus.dir = true;
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
        if(player.shootTimer == 0 && player.shotCount < playerStatus.maxShotCount) {
            bulletAdd(ent->x + (playerStatus.dir ? 8 * 16 : -8 * 16), ent->y, playerStatus.dir, BULLET_TYPE_FIREBALL);
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
            if(player.fallTimer < 255) {
                player.fallTimer++;
            }
        } else {
            if(!player.landed) {
                ent->controlFlags &= ~ENT_CTRL_FLAG_IGNORE_SLOPES;
                player.landed = true;
                ent->yspd = 0;

                if(player.fallTimer > 16) {                    
                    particleStarAdd(ent->x + 10 * 16, ent->y + 4 * 16);
                }
                player.fallTimer = 0;
            }
        }
    }

    if(!player.landed) {
        ent->controlFlags |= ENT_CTRL_FLAG_IGNORE_SLOPES;
    }
    
    if(moved) {
        player.timer++;
        if(player.timer >= 16) {
            player.timer = 0;    
        }
    } else {
        player.timer = 0;
    }

    if(playerStatus.dir) {
        ent->drawFlags |= ENT_DRAW_FLAG_HFLIP;
    } else {
        ent->drawFlags &= ~ENT_DRAW_FLAG_HFLIP;
    }

    if(player.jumpTimer != 0 || player.fallTimer > 0) {
        ent->sprite = SPRITE_TYPE_PLAYER_2;
    } else {
        ent->sprite = player.timer < 1 || player.timer >= 9 ? SPRITE_TYPE_PLAYER_1 : SPRITE_TYPE_PLAYER_2;
    }
}

void playerHurt() {
    if(player.hurtTimer == 0) {
        Entity* ent = &ents[ENT_OFFSET_PLAYER];

        if(playerStatus.hp > 0) {
            playerStatus.hp--;
        }

        player.hurtTimer = PLAYER_HURT_DURATION;
        if(playerStatus.dir) {
            ent->xspd = -40;            
        } else {
            ent->xspd = 40;
        }
        ent->yspd = 0;
        player.jumpTimer = 0;
    }
}

void playerDraw() {
    entityDraw(ENT_OFFSET_PLAYER);
}

void playerDrawHUD() {
    uint8_t h;
    for(h = 0; h < playerStatus.maxHP; ++h) {
        for(uint8_t i = 0; i < 3; ++i) {
            for(uint8_t j = 0; j < 3; ++j) {
                frogboy::drawTile(1 + h * 8 + i, 1 + j, spritesBitmap, 0x40, 0, false, false);
            }
        }
    }
    for(h = 0; h < playerStatus.hp; ++h) {
        frogboy::drawTile(2 + h * 8, 2, spritesBitmap, 0x40, 1, false, false);
    }
    for(; h < playerStatus.maxHP; ++h) {
        frogboy::drawTile(2 + h * 8, 2, spritesBitmap, 0x41, 1, false, false);
    }    
}
