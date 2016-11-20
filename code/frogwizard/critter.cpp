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
#include "game.h"
#include "text.h"

enum {
    CRITTER_SPAWN_MAX = 64,
    CRITTER_SPAWN_BIT_ARRAY_SIZE = (CRITTER_SPAWN_MAX + 7) / 8,
};

Critter critters[ENT_COUNT_CRITTER];
uint8_t critterSpawnUsed[CRITTER_SPAWN_BIT_ARRAY_SIZE];
uint8_t critterSpawnKilled[CRITTER_SPAWN_BIT_ARRAY_SIZE];
int16_t critterSpawnHibernateX[CRITTER_SPAWN_MAX];
int16_t critterSpawnHibernateY[CRITTER_SPAWN_MAX];

typedef void (*CritterHandler)(Entity* ent, Critter* critter);
extern const uint8_t critterMaxHP[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const uint8_t critterLayer[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const bool critterCanHibernate[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const CritterHandler critterInitHandler[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const CritterHandler critterUpdateHandler[CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;

extern const uint8_t critterSpawnCount[MAP_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const uint8_t* const critterSpawnData[MAP_TYPE_COUNT] FROGBOY_ROM_DATA;

void critterInitSystem() {
    memset(critters, 0, sizeof(critters));
    memset(critterSpawnUsed, 0, sizeof(critterSpawnUsed));
    memset(critterSpawnKilled, 0, sizeof(critterSpawnKilled));
    memset(critterSpawnKilled, 0, sizeof(critterSpawnKilled));
    for(uint8_t i = 0; i != CRITTER_SPAWN_MAX; ++i) {
        critterSpawnHibernateX[i] = critterSpawnHibernateY[i] = 0x7FFF;
    }
}

Critter* critterAdd(int16_t x, int16_t y, CritterType type, uint8_t data) {
    Entity* ent = entityAdd(x, y, ENT_OFFSET_CRITTER, ENT_COUNT_CRITTER);

    if(ent != nullptr) {
        ptrdiff_t critterIndex = ent - &ents[ENT_OFFSET_CRITTER];
        Critter* critter = &critters[critterIndex];

        memset(critter, 0, sizeof(Critter));
        critter->type = type;
        critter->hp = frogboy::readRom<uint8_t>(&critterMaxHP[critter->type]);
        critter->data = data;
        critter->spawnIndex = 0xFF;
        frogboy::readRom<CritterHandler>(&critterInitHandler[critter->type])(ent, critter);
        return critter;
    }

    return nullptr;
}

void critterRemove(Entity* ent, Critter* critter) {
    critter->type = 0;

    uint8_t spawnIndex = critter->spawnIndex;
    if(spawnIndex != 0xFF) {
        uint8_t mask = 1 << (spawnIndex % 8);
        critterSpawnUsed[spawnIndex / 8] &= ~mask;
        critterSpawnKilled[spawnIndex / 8] |= mask;
        critterSpawnHibernateX[spawnIndex] = ent->x / 16;
        critterSpawnHibernateY[spawnIndex] = ent->y / 16;
    }

    entityRemove(ent);
}

void critterHurt(Entity* ent, Critter* critter, uint8_t damage) {
    if(critter->hp > damage) {
        frogboy::playTone(300, 6);
        critter->hp -= damage;
        critter->flashTimer = 16;
        ent->drawFlags |= ENT_DRAW_FLAG_FLASH;
    } else {
        frogboy::playTone(300, 15);
        particleAdd(ent->x + 4 * 16, ent->y + 6 * 16, -4, -4, 0x52, 20);
        particleAdd(ent->x + 4 * 16, ent->y + 6 * 16, 4, -4, 0x52, 20);
        particleAdd(ent->x + 4 * 16, ent->y + 6 * 16, -4, 4, 0x52, 20);
        particleAdd(ent->x + 4 * 16, ent->y + 6 * 16, 4, 4, 0x52, 20);
        critterRemove(ent, critter);
    }
}

static void updateSpawnPoints() {
    uint8_t count = frogboy::readRom<uint8_t>(&critterSpawnCount[mapCurrentIndex]);
    const uint8_t* spawnPtr = frogboy::readRom<const uint8_t*>(&critterSpawnData[mapCurrentIndex]);

    for(uint8_t i = 0; i != count; ++i) {
        int16_t x = static_cast<int16_t>(frogboy::readRom<uint8_t>(spawnPtr++)) * 16;
        int16_t y = static_cast<int16_t>(frogboy::readRom<uint8_t>(spawnPtr++)) * 16;
        uint8_t type = frogboy::readRom<uint8_t>(spawnPtr++);
        uint8_t data = frogboy::readRom<uint8_t>(spawnPtr++);
        uint8_t mask = 1 << (i % 8);

        if(frogboy::readRom<bool>(&critterCanHibernate[type])
        && critterSpawnHibernateX[i] != 0x7FFF) {
            x = critterSpawnHibernateX[i];
            y = critterSpawnHibernateY[i];
        }

        if(gameCheckOnScreen(x + 8, y + 8, 64, 16)) {
            if((critterSpawnUsed[i / 8] & mask) == 0
            && (critterSpawnKilled[i / 8] & mask) == 0) {
                Critter* critter = critterAdd(x * 16, y * 16, static_cast<CritterType>(type), data);
                if(critter != nullptr) {
                    critter->spawnIndex = i;
                    critterSpawnUsed[i / 8] |= mask;
                }
            }
        } else {
            critterSpawnKilled[i / 8] &= ~mask;
        }
    }
}

void critterUpdateAll() {
    updateSpawnPoints();

    Critter* critter = &critters[0];
    Critter* critterEnd = critter + ENT_COUNT_CRITTER;
    Entity* ent = &ents[ENT_OFFSET_CRITTER];

    for(; critter != critterEnd; ++critter, ++ent) {
        if((ent->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
            entityUpdate(ent);

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

            if(!gameCheckOnScreen(ent->x / 16 + 8, ent->y / 16 + 8, 112, 16)) {
                critterRemove(ent, critter);
            }
        }
    }
}

void critterDrawAll() {
    for(uint8_t layer = 0; layer != 2; ++layer) {
        Critter* critter = &critters[0];
        Critter* critterEnd = critter + ENT_COUNT_CRITTER;
        Entity* ent = &ents[ENT_OFFSET_CRITTER];

        for(; critter != critterEnd; ++critter, ++ent) {
            if((ent->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
                if(frogboy::readRom<uint8_t>(&critterLayer[critter->type]) == layer) {
                    entityDraw(ent);
                }
            }
        }
    }
}

void coinInit(Entity* ent, Critter* critter) {}
void coinUpdate(Entity* ent, Critter* critter) {}

enum {
    WALKER_ACCEL = 2,
    WALKER_MAX_SPEED = 8,
};

void walkerInit(Entity* ent, Critter* critter) {
    Entity* playerEnt = &ents[ENT_OFFSET_PLAYER];

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

    if(entityCollide(ent, -1, &ents[ENT_OFFSET_PLAYER], -1)) {
        playerHurt();
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
    if(!playerStatus.usedDoor
    && entityCollide(ent, -1, &ents[ENT_OFFSET_PLAYER], -1)
    && frogboy::isPressed(frogboy::BUTTON_UP)) {
        frogboy::playTone(390, 10);
        playerStatus.usedDoor = true;
        playerStatus.nextMap = critter->data;
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

        return hitboxCollide(ent->x / 16,
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
    Entity* playerEnt = &ents[ENT_OFFSET_PLAYER];

    if(blockCheckPushed(ent, playerEnt)) {
        player.pushing = true;
        if(critter->var[0] >= BLOCK_INERTIA_DURATION) {
            ent->xspd = playerEnt->xspd;
        } else {
            critter->var[0]++;
        }
    } else {        
        ent->xspd = ent->xspd * BLOCK_FRICTION_MULTIPLIER / BLOCK_FRICTION_DIVISOR;
        critter->var[0] = 0;
    }

    if(!entityDetectFloor(ent)) {
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
    ent->sprite = critter->data;
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
    if(entityCollide(ent, -1, &ents[ENT_OFFSET_PLAYER], -1)) {
        frogboy::playTone(600, 10);
        entityRemove(ent);

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

const uint8_t FROGBOY_ROM_DATA grasslandSpawnData[] = {
    10, 1, CRITTER_TYPE_DOOR, DOOR_TYPE_GRASSLAND_HOUSE,
    16, 2, CRITTER_TYPE_BLOCK, 0,
    24, 1, CRITTER_TYPE_WALKER, 0,
    49, 1, CRITTER_TYPE_BLOCK, 0,
    40, 1, CRITTER_TYPE_DOOR, DOOR_TYPE_GRASSLAND_HOUSE2,
    67, 2, CRITTER_TYPE_DOOR, DOOR_TYPE_GRASSLAND_HOUSE3,
};

const uint8_t houseSpawnData[] FROGBOY_ROM_DATA = {
    2, 2, CRITTER_TYPE_DOOR, DOOR_TYPE_HOUSE_GRASSLAND,
    2, 1, CRITTER_TYPE_LABEL, TEXT_TYPE_FIND_EGG,
    4, 2, CRITTER_TYPE_EGG, TEXT_TYPE_FIND_EGG,
};

const uint8_t house2SpawnData[] FROGBOY_ROM_DATA = {
    5, 2, CRITTER_TYPE_DOOR, DOOR_TYPE_HOUSE2_GRASSLAND,
};

const uint8_t house3SpawnData[] FROGBOY_ROM_DATA = {
    2, 2, CRITTER_TYPE_DOOR, DOOR_TYPE_HOUSE3_GRASSLAND,
};

const uint8_t critterSpawnCount[MAP_TYPE_COUNT] FROGBOY_ROM_DATA = {
    sizeof(grasslandSpawnData) / 4,
    sizeof(houseSpawnData) / 4,
    sizeof(house2SpawnData) / 4,
    sizeof(house3SpawnData) / 4,
};

const uint8_t* const critterSpawnData[MAP_TYPE_COUNT] FROGBOY_ROM_DATA = {
    grasslandSpawnData,
    houseSpawnData,
    house2SpawnData,
    house3SpawnData,
};
