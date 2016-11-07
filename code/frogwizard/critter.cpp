#include <string.h>
#include "frogboy.h"
#include "critter.h"
#include "sprite.h"
#include "player.h"
#include "hitbox.h"

Critter critters[ENT_COUNT_CRITTER];

typedef void (*CritterHandler)(uint8_t critterIndex);
extern const uint8_t critterMaxHP[(int) CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const CritterHandler critterInitHandler[(int) CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const CritterHandler critterUpdateHandler[(int) CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA;

void critterInitSystem() {
    memset(critters, 0, sizeof(critters));
}

uint8_t critterAdd(int16_t x, int16_t y, CritterType type) {
    uint8_t entityIndex = entityAdd(x, y, ENT_OFFSET_CRITTER, ENT_COUNT_CRITTER);

    if(entityIndex != 0xFF) {
        uint8_t critterIndex = entityIndex - ENT_OFFSET_CRITTER;
        Critter* critter = &critters[critterIndex];

        memset(critter, 0, sizeof(Critter));
        critter->type = (uint8_t) type;
        frogboy::readRom<CritterHandler>(&critterInitHandler[critter->type])(critterIndex);
        return critterIndex;
    }

    return 0xFF;
}

void critterRemove(uint8_t critterIndex) {
    critters[critterIndex].type = 0;
    entityRemove(ENT_OFFSET_CRITTER + critterIndex);
}

void critterUpdateAll() {
    for(uint8_t critterIndex = 0; critterIndex != ENT_COUNT_CRITTER; ++critterIndex) {
        uint8_t entityIndex = ENT_OFFSET_CRITTER + critterIndex;
        Entity* ent = &ents[entityIndex];

        if((ent->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
            entityUpdate(entityIndex);

            Critter* critter = &critters[critterIndex];
            frogboy::readRom<CritterHandler>(&critterUpdateHandler[critter->type])(critterIndex);
        }
    }
}

void critterDrawAll() {
    for(uint8_t critterIndex = 0; critterIndex != ENT_COUNT_CRITTER; ++critterIndex) {
        entityDraw(ENT_OFFSET_CRITTER + critterIndex);
    }
}

void coinInit(uint8_t critterIndex) {}
void coinUpdate(uint8_t critterIndex) {}

enum {
    WALKER_ACCEL = 2,
    WALKER_MAX_SPEED = 16,
};

void walkerInit(uint8_t critterIndex) {
    uint8_t entityIndex = ENT_OFFSET_CRITTER + critterIndex;
    Entity* ent = &ents[entityIndex];
    Critter* critter = &critters[critterIndex];

    ent->hitbox = HITBOX_TYPE_HUMAN_16x16;
    ent->xspd = critter->var[1] ? WALKER_MAX_SPEED : -WALKER_MAX_SPEED;
    ent->sprite = (uint8_t) SPRITE_TYPE_WALKER_1;
}

void walkerUpdate(uint8_t critterIndex) {
    uint8_t entityIndex = ENT_OFFSET_CRITTER + critterIndex;
    Entity* ent = &ents[entityIndex];
    Critter* critter = &critters[critterIndex];

    critter->var[0]++;
    if(critter->var[0] > 8) {
        critter->var[0] = 0;
        ent->sprite = ent->sprite == (uint8_t) SPRITE_TYPE_WALKER_2
            ? (uint8_t) SPRITE_TYPE_WALKER_1
            : (uint8_t) SPRITE_TYPE_WALKER_2;
    }
    
    ent->xspd += critter->var[1] ? WALKER_ACCEL : -WALKER_ACCEL;
    if(ent->xspd < -WALKER_MAX_SPEED) {
        ent->xspd = -WALKER_MAX_SPEED;
    }
    if(ent->xspd > WALKER_MAX_SPEED) {
        ent->xspd = WALKER_MAX_SPEED;
    }    

    if((ent->status & ENT_STATUS_HIT_OBS_X) != 0) {
        critter->var[1] ^= 1;
        ent->drawFlags ^= ENT_DRAW_FLAG_HFLIP;
    }

    if(entityCollide(entityIndex, -3, ENT_OFFSET_PLAYER, -3)) {
        playerHurt();
    }
}

const uint8_t critterMaxHP[(int) CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA = {
    0,
    1,
};

const CritterHandler critterInitHandler[(int) CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA = {
    coinInit,
    walkerInit,
};

const CritterHandler critterUpdateHandler[(int) CRITTER_TYPE_COUNT] FROGBOY_ROM_DATA = {
    coinUpdate,
    walkerUpdate,
};

