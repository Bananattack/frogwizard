#include <string.h>

#include "frogboy.h"
#include "player.h"
#include "sprite.h"
#include "bullet.h"
#include "hitbox.h"
#include "particle.h"
#include "sprites_bitmap.h"

namespace {
    enum {
        PLAYER_MAX_HP_START = 3,

        PLAYER_RUN_XACCEL = 6,
        PLAYER_RUN_MAX_XSPEED = 20,

        PLAYER_PUSH_MAX_XSPEED = 16,

        PLAYER_FRICTION_MULTIPLIER = 14,
        PLAYER_FRICTION_DIVISOR = 16,

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
}

Player player;

void Player::initSystem() {
    memset(&player.status, 0, sizeof(PlayerStatus));
    player.status.hp = PLAYER_MAX_HP_START;
    player.status.maxHP = PLAYER_MAX_HP_START;
    player.status.maxShotCount = PLAYER_SHOOT_MAX_SHOTS;
    player.status.nextMap = 0xFF;
    player.status.dir = true;
    player.status.usedDoor = false;
}

void Player::add(int16_t x, int16_t y) {
    Entity* ent = Entity::add(x, y, ENT_OFFSET_PLAYER, ENT_COUNT_PLAYER);

    if(ent != nullptr) {
        memset(&player.instance, 0, sizeof(PlayerInstance));

        player.instance.landed = true;
        ent->sprite = SPRITE_TYPE_PLAYER_1;
        ent->hitbox = HITBOX_TYPE_HUMAN_16x16;
        if(player.status.dir) {
            ent->drawFlags |= ENT_DRAW_FLAG_HFLIP;
        }
    }
}

void Player::update() {
    bool moved = false;
    Entity* ent = &Entity::data[ENT_OFFSET_PLAYER];

    ent->update();

    bool left = frogboy::isPressed(frogboy::BUTTON_LEFT);
    bool right = frogboy::isPressed(frogboy::BUTTON_RIGHT);
    if(left && right) {
        left = right = false;
    }

    if(status.usedDoor && !frogboy::isPressed(frogboy::BUTTON_UP)) {
        status.usedDoor = false;
    }

    ent->drawFlags &= ~ENT_DRAW_FLAG_HIDDEN;
    if(instance.hurtTimer > 0) {
        instance.hurtTimer--;
        if(instance.hurtTimer % (PLAYER_HURT_BLINK_INTERVAL * 2) < PLAYER_HURT_BLINK_INTERVAL) {
            ent->drawFlags |= ENT_DRAW_FLAG_HIDDEN;
        }
        if(instance.hurtTimer > PLAYER_HURT_DURATION - PLAYER_HURT_STOP_CONTROL_DURATION)  {
            left = right = false;
        }
    }
    
    int16_t maxSpeed = PLAYER_RUN_MAX_XSPEED;
    if(instance.pushing) {
        maxSpeed = PLAYER_PUSH_MAX_XSPEED;
    }

    if(left) {
        ent->xspd -= PLAYER_RUN_XACCEL;
        if(ent->xspd < -maxSpeed) {
            ent->xspd = -maxSpeed;
        }        
        status.dir = false;
        moved = true;
    }
    else if(right) {
        ent->xspd += PLAYER_RUN_XACCEL;
        if(ent->xspd > maxSpeed) {
            ent->xspd = maxSpeed;
        }
        status.dir = true;
        moved = true;
    } else {
        ent->xspd = ent->xspd * PLAYER_FRICTION_MULTIPLIER / PLAYER_FRICTION_DIVISOR;
    }

    if(instance.shootTimer > 0) {
        instance.shootTimer--;
    }

    if(frogboy::isPressed(frogboy::BUTTON_SHOOT)) {
        if(!instance.shootPressed) {
            instance.bufferShootTimer = PLAYER_SHOOT_INPUT_BUFFER_TIME;
            instance.shootPressed = true;
        }
    } else {
        instance.shootPressed = false;
        if(instance.bufferShootTimer > 0) {
            instance.bufferShootTimer--;
        }
    }

    if(instance.bufferShootTimer > 0) {
        if(instance.shootTimer == 0 && instance.shotCount < status.maxShotCount) {
            Bullet::add(ent->x + (status.dir ? 8 * 16 : -8 * 16), ent->y, status.dir, BULLET_TYPE_FIREBALL);
            instance.bufferShootTimer = 0;
            instance.shootTimer = PLAYER_SHOOT_COOLDOWN;
            instance.shotCount++;
            frogboy::playTone(500, 16);
        }
    }

    if(frogboy::isPressed(frogboy::BUTTON_JUMP)) {
        if(!instance.jumpPressed) {
            instance.bufferJumpTimer = PLAYER_JUMP_INPUT_BUFFER_TIME;
            instance.jumpPressed = true;
        }
    } else {
        instance.jumpPressed = false;
        if(instance.bufferJumpTimer > 0) {
            instance.bufferJumpTimer--;
        }
        if(instance.jumpTimer <= PLAYER_JUMP_MAX_DURATION - PLAYER_JUMP_MIN_DURATION) {
            instance.jumpTimer = 0;
        }        
    }

    if(instance.bufferJumpTimer > 0) {
        if(instance.landed) {
            instance.bufferJumpTimer = 0;
            instance.landed = false;
            instance.jumpTimer = PLAYER_JUMP_MAX_DURATION;
            frogboy::playTone(250, 6);
        }
    }

    if(instance.jumpTimer > 0) {
        instance.landed = false;
        instance.jumpTimer--;
        ent->yspd -= PLAYER_JUMP_YACCEL;
        if(ent->yspd < -PLAYER_JUMP_MAX_YSPEED) {
            ent->yspd = -PLAYER_JUMP_MAX_YSPEED;
        }
    }

    if(instance.jumpTimer == 0) {
        if(!ent->detectFloor()) {
            ent->yspd += PLAYER_FALL_YACCEL;
            if(ent->yspd > PLAYER_FALL_MAX_YSPEED) {
                ent->yspd = PLAYER_FALL_MAX_YSPEED;
            }
            instance.landed = false;
            if(instance.fallTimer < 255) {
                instance.fallTimer++;
            }
        } else {
            if(!instance.landed) {
                ent->controlFlags &= ~ENT_CTRL_FLAG_IGNORE_SLOPES;
                instance.landed = true;
                ent->yspd = 0;

                if(instance.fallTimer > 4) {                    
                    Particle::addStar(ent->x + 10 * 16, ent->y + 4 * 16);
                    frogboy::playTone(200, 10);
                }
                instance.fallTimer = 0;
            }
        }
    }

    if(!instance.landed) {
        ent->controlFlags |= ENT_CTRL_FLAG_IGNORE_SLOPES;
    }
    
    if(moved) {
        instance.moveTimer++;
        if(instance.moveTimer >= 16) {
            instance.moveTimer = 0;    
        }
    } else {
        instance.moveTimer = 0;
    }

    if(status.dir) {
        ent->drawFlags |= ENT_DRAW_FLAG_HFLIP;
    } else {
        ent->drawFlags &= ~ENT_DRAW_FLAG_HFLIP;
    }

    if(instance.jumpTimer != 0 || instance.fallTimer > 0) {
        ent->sprite = SPRITE_TYPE_PLAYER_2;
    } else {
        ent->sprite = instance.moveTimer < 1 || instance.moveTimer >= 9 ? SPRITE_TYPE_PLAYER_1 : SPRITE_TYPE_PLAYER_2;
    }

    instance.pushing = false;
}

void Player::hurt() {
    if(instance.hurtTimer == 0) {
        Entity* ent = &Entity::data[ENT_OFFSET_PLAYER];
        frogboy::playTone(890, 20);

        if(status.hp > 0) {
            status.hp--;
        }

        instance.hurtTimer = PLAYER_HURT_DURATION;
        if(status.dir) {
            ent->xspd = -40;            
        } else {
            ent->xspd = 40;
        }
        ent->yspd = 0;
        instance.jumpTimer = 0;
    }
}

void Player::draw() const {
    Entity::data[ENT_OFFSET_PLAYER].draw();
}

void Player::drawHUD() const {
    uint8_t h;
    for(h = 0; h < status.maxHP; ++h) {
        for(uint8_t i = 0; i < 3; ++i) {
            for(uint8_t j = 0; j < 3; ++j) {
                frogboy::drawTile(1 + h * 8 + i, 1 + j, spritesBitmap, 0x40, 0, false, false);
            }
        }
    }
    for(h = 0; h < status.hp; ++h) {
        frogboy::drawTile(2 + h * 8, 2, spritesBitmap, 0x40, 1, false, false);
    }
    for(; h < status.maxHP; ++h) {
        frogboy::drawTile(2 + h * 8, 2, spritesBitmap, 0x41, 1, false, false);
    }    
}
