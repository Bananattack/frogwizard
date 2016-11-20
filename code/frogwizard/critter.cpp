#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "frogboy.h"
#include "critter.h"
#include "sprite.h"
#include "player.h"
#include "particle.h"
#include "map.h"
#include "hitbox.h"
#include "camera.h"
#include "spawn.h"

typedef void (*CritterHandler)(Entity* ent, Critter* critter);
extern const uint8_t critterMaxHP[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const uint8_t critterLayer[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const bool critterCanHibernate[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const CritterHandler critterInitHandler[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const CritterHandler critterUpdateHandler[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;

Critter Critter::data[ENT_COUNT_CRITTER];

void Critter::initSystem() {
    memset(data, 0, sizeof(data));
}

Critter* Critter::add(int16_t x, int16_t y, CritterType type, uint8_t metadata) {
    Entity* ent = Entity::add(x, y, ENT_OFFSET_CRITTER, ENT_COUNT_CRITTER);

    if(ent != nullptr) {
        ptrdiff_t critterIndex = ent - &Entity::data[ENT_OFFSET_CRITTER];
        Critter* critter = &data[critterIndex];

        memset(critter, 0, sizeof(Critter));
        critter->type = type;
        critter->hp = frogboy::readRom<uint8_t>(&critterMaxHP[critter->type]);
        critter->metadata = metadata;
        critter->spawnIndex = 0xFF;
        frogboy::readRom<CritterHandler>(&critterInitHandler[critter->type])(ent, critter);
        return critter;
    }

    return nullptr;
}

bool Critter::canPersistType(CritterType type) {
    return frogboy::readRom<bool>(&critterCanHibernate[type]);
}

void Critter::updateAll() {
    Critter* critter = &data[0];
    Critter* critterEnd = critter + ENT_COUNT_CRITTER;
    Entity* ent = &Entity::data[ENT_OFFSET_CRITTER];

    for(; critter != critterEnd; ++critter, ++ent) {
        if((ent->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
            ent->update();

            frogboy::readRom<CritterHandler>(&critterUpdateHandler[critter->type])(ent, critter);

            ent->drawFlags &= ~ENT_DRAW_FLAG_HIDDEN;
            if(critter->flashTimer > 0) {
                critter->flashTimer--;
                if(critter->flashTimer % 8 < 4) {
                    ent->drawFlags |= ENT_DRAW_FLAG_HIDDEN;
                }
            } else {
                ent->drawFlags &= ~ENT_DRAW_FLAG_FLASH;
            }

            if(!camera.containsPoint(ent->x / 16 + 8, ent->y / 16 + 8, 112, 16)) {
                critter->remove(ent);
            }
        }
    }
}

void Critter::drawAll() {
    for(uint8_t layer = 0; layer != 2; ++layer) {
        Critter* critter = &data[0];
        Critter* critterEnd = critter + ENT_COUNT_CRITTER;
        Entity* ent = &Entity::data[ENT_OFFSET_CRITTER];

        for(; critter != critterEnd; ++critter, ++ent) {
            if((ent->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
                if(frogboy::readRom<uint8_t>(&critterLayer[critter->type]) == layer) {
                    ent->draw();
                }
            }
        }
    }
}

void Critter::remove(Entity* ent) {
    type = 0;
    spawn::suspend(spawnIndex, ent->x / 16, ent->y / 16);
    ent->remove();
}

void Critter::hurt(Entity* ent, uint8_t damage) {
    if(hp > damage) {
        frogboy::playTone(300, 6);
        hp -= damage;
        flashTimer = 16;
        ent->drawFlags |= ENT_DRAW_FLAG_FLASH;
    } else {
        frogboy::playTone(300, 15);
        Particle::add(ent->x + 4 * 16, ent->y + 6 * 16, -4, -4, 0x52, 20);
        Particle::add(ent->x + 4 * 16, ent->y + 6 * 16, 4, -4, 0x52, 20);
        Particle::add(ent->x + 4 * 16, ent->y + 6 * 16, -4, 4, 0x52, 20);
        Particle::add(ent->x + 4 * 16, ent->y + 6 * 16, 4, 4, 0x52, 20);
        remove(ent);
    }
}

void coinInit(Entity* ent, Critter* critter) {}
void coinUpdate(Entity* ent, Critter* critter) {}

enum {
    WALKER_ACCEL = 2,
    WALKER_MAX_SPEED = 8,
};

void walkerInit(Entity* ent, Critter* critter) {
    Entity* playerEnt = &Entity::data[ENT_OFFSET_PLAYER];

    ent->hitbox = HITBOX_TYPE_HUMAN_16x16;
    ent->controlFlags |= ENT_CTRL_FLAG_BULLET_TARGET;

    bool faceLeft = playerEnt->x < ent->x;
    ent->xspd = faceLeft ? WALKER_MAX_SPEED : -WALKER_MAX_SPEED;
    ent->drawFlags |= faceLeft ? 0 : ENT_DRAW_FLAG_HFLIP;       
    ent->sprite = SPRITE_TYPE_WALKER_1;
    critter->var[1] = faceLeft ? 0 : 1;
}

void walkerUpdate(Entity* ent, Critter* critter) {
    critter->var[0]++;
    if(critter->var[0] > 12) {
        critter->var[0] = 0;
        ent->sprite = ent->sprite == SPRITE_TYPE_WALKER_2
            ? SPRITE_TYPE_WALKER_1
            : SPRITE_TYPE_WALKER_2;
    }
    
    ent->xspd += critter->var[1] != 0 ? WALKER_ACCEL : -WALKER_ACCEL;
    if(ent->xspd < -WALKER_MAX_SPEED) {
        ent->xspd = -WALKER_MAX_SPEED;
    }
    if(ent->xspd > WALKER_MAX_SPEED) {
        ent->xspd = WALKER_MAX_SPEED;
    }

    ent->yspd = 8;

    if(critter->var[3] < 255) {
        critter->var[3]++;
    }

    ent->drawFlags &= ~ENT_DRAW_FLAG_HFLIP;
    if(critter->var[1] != 0) {
        ent->drawFlags |= ENT_DRAW_FLAG_HFLIP;
    }

    if((ent->status & ENT_STATUS_HIT_OBS_X) != 0
    && critter->var[3] > 8) {
        critter->var[1] ^= 1;
        critter->var[3] = 0;
    }

    if(ent->collide(-1, &Entity::data[ENT_OFFSET_PLAYER], -1)) {
        player.hurt();
    }
}

void doorInit(Entity* ent, Critter* critter) {
    static_cast<void>(critter);
    ent->hitbox = HITBOX_TYPE_HUMAN_16x16;
    ent->sprite = SPRITE_TYPE_DOOR;
    ent->collisionCategory = COLLISION_CATEGORY_ZONE;
    ent->controlFlags |= ENT_CTRL_FLAG_IGNORE_OBS | ENT_CTRL_FLAG_IGNORE_SLOPES;
}

void doorUpdate(Entity* ent, Critter* critter) {
    if(!player.status.usedDoor
    && ent->collide(-1, &Entity::data[ENT_OFFSET_PLAYER], -1)
    && frogboy::isPressed(frogboy::BUTTON_UP)) {
        frogboy::playTone(390, 10);
        player.status.usedDoor = true;
        player.status.nextMap = critter->metadata;
    }
}

enum {
    BLOCK_DETECTION_X_RANGE = 4,
    BLOCK_DETECTION_SLOPE_Y_RANGE = 4,
    BLOCK_FRICTION_MULTIPLIER = 31,
    BLOCK_FRICTION_DIVISOR = 32,
    BLOCK_INERTIA_DURATION = 12,
    BLOCK_FALL_YACCEL = 2,
    BLOCK_FALL_MAX_YSPEED = 32,
};

void blockInit(Entity* ent, Critter* critter) {
    static_cast<void>(critter);
    ent->hitbox = HITBOX_TYPE_BLOCK;
    ent->collisionCategory = COLLISION_CATEGORY_BLOCK;
    ent->sprite = SPRITE_TYPE_BLOCK;
}

static bool blockCheckPushed(Entity* ent, Entity* playerEnt) {
    if((playerEnt->xspd < 0 || playerEnt->xspd > 0)
    && (frogboy::isPressed(frogboy::BUTTON_LEFT) || frogboy::isPressed(frogboy::BUTTON_RIGHT))) {
        int16_t x = (playerEnt->x + playerEnt->xspd * BLOCK_DETECTION_X_RANGE) / 16;
        int16_t y = playerEnt->y / 16;
        if((ent->status & ENT_STATUS_DOWNHILL) != 0) {
            y += BLOCK_DETECTION_SLOPE_Y_RANGE;
        } else if((ent->status & ENT_STATUS_UPHILL) != 0) {
            y -= BLOCK_DETECTION_SLOPE_Y_RANGE;
        }

        return hitbox::collide(ent->x / 16,
            ent->y / 16,
            (HitboxType) ent->hitbox,
            0,
            x,
            y,
            (HitboxType) HITBOX_TYPE_PUSH_DETECTION,
            0);
    }
    return false;
}

void blockUpdate(Entity* ent, Critter* critter) {
    Entity* playerEnt = &Entity::data[ENT_OFFSET_PLAYER];

    if(blockCheckPushed(ent, playerEnt)) {
        player.instance.pushing = true;
        if(critter->var[0] >= BLOCK_INERTIA_DURATION) {
            ent->xspd = playerEnt->xspd;
        } else {
            critter->var[0]++;
        }
    } else {        
        ent->xspd = ent->xspd * BLOCK_FRICTION_MULTIPLIER / BLOCK_FRICTION_DIVISOR;
        critter->var[0] = 0;
    }

    if(!ent->detectFloor()) {
        ent->yspd += BLOCK_FALL_YACCEL;
        if(ent->yspd > BLOCK_FALL_MAX_YSPEED) {
            ent->yspd = BLOCK_FALL_MAX_YSPEED;
        }
    } else {
        if(ent->yspd != 0) {
            if(ent->yspd >= BLOCK_FALL_MAX_YSPEED / 6) {
                frogboy::playTone(150, 16);
            }
            ent->yspd = 0;
        }
    }
}

void labelInit(Entity* ent, Critter* critter) {
    ent->collisionCategory = COLLISION_CATEGORY_ZONE;
    ent->controlFlags |= ENT_CTRL_FLAG_IGNORE_OBS | ENT_CTRL_FLAG_IGNORE_SLOPES;
    ent->drawFlags |= ENT_DRAW_FLAG_TEXT;
    ent->sprite = critter->metadata;
}

void labelUpdate(Entity* ent, Critter* critter) {}

void eggInit(Entity* ent, Critter* critter) {
    static_cast<void>(critter);
    ent->hitbox = HITBOX_TYPE_16x16;
    ent->sprite = SPRITE_TYPE_EGG;
    ent->collisionCategory = COLLISION_CATEGORY_ZONE;
    ent->controlFlags |= ENT_CTRL_FLAG_IGNORE_OBS | ENT_CTRL_FLAG_IGNORE_SLOPES;

    // TODO: despawn immediately if collected already
}

void eggUpdate(Entity* ent, Critter* critter) {
    if(ent->collide(-1, &Entity::data[ENT_OFFSET_PLAYER], -1)) {
        frogboy::playTone(600, 10);
        ent->remove();

        // TODO: track collection
    }
}

const uint8_t critterMaxHP[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA = {
    0,
    3,
    0,
    0,
    0,
};

const uint8_t critterLayer[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA = {
    1,
    1,
    0,
    1,
    0,
};

const bool critterCanHibernate[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA = {
    false,
    false,
    false,
    true,
    false,
};

const CritterHandler critterInitHandler[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA = {
    coinInit,
    walkerInit,
    doorInit,
    blockInit,
    labelInit,
    eggInit,
};

const CritterHandler critterUpdateHandler[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA = {
    coinUpdate,
    walkerUpdate,
    doorUpdate,
    blockUpdate,
    labelUpdate,
    eggUpdate,
};
