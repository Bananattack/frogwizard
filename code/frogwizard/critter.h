#ifndef CRITTER_H
#define CRITTER_H

#include "entity.h"

enum CritterType {
    CRITTER_TYPE_COIN,
    CRITTER_TYPE_WALKER,
    CRITTER_TYPE_COUNT,
};

struct Critter {
    uint8_t type;
    uint8_t flags;
    uint8_t hp;
    uint8_t flashTimer;
    uint8_t var[6];
};

void critterInitSystem();
uint8_t critterAdd(int16_t x, int16_t y, CritterType type);
void critterRemove(uint8_t critterIndex);
void critterHurt(uint8_t critterIndex, uint8_t damage);
void critterUpdateAll();
void critterDrawAll();

#endif
