#ifndef CRITTER_H
#define CRITTER_H

#include "entity.h"

enum CritterType {
    CRITTER_TYPE_COIN,
    CRITTER_TYPE_WALKER,
    CRITTER_TYPE_DOOR,
    CRITTER_TYPE_COUNT,
};

struct Critter {
    uint8_t type;
    uint8_t data;
    uint8_t hp;
    uint8_t spawnIndex;
    uint8_t flashTimer;
    uint8_t var[5];
};

extern Critter critters[ENT_COUNT_CRITTER];

void critterInitSystem();
Critter* critterAdd(int16_t x, int16_t y, CritterType type, uint8_t data);
void critterRemove(Entity* ent, Critter* critter);
void critterHurt(Entity* ent, Critter* critter, uint8_t damage);
void critterUpdateAll();
void critterDrawAll();

#endif
