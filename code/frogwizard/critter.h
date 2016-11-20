#ifndef CRITTER_H
#define CRITTER_H

#include "entity.h"

enum CritterType {
    CRITTER_TYPE_COIN,
    CRITTER_TYPE_WALKER,
    CRITTER_TYPE_DOOR,
    CRITTER_TYPE_BLOCK,
    CRITTER_TYPE_LABEL,
    CRITTER_TYPE_EGG,
    CRITTER_TYPE_COUNT,
};

struct Critter {
    uint8_t type;
    uint8_t metadata;
    uint8_t hp;
    uint8_t spawnIndex;
    uint8_t flashTimer;
    uint8_t var[5];

    static Critter data[ENT_COUNT_CRITTER];

    static void initSystem();
    static Critter* add(int16_t x, int16_t y, CritterType type, uint8_t data);
    static void updateAll();
    static void drawAll();
    static bool canPersist(CritterType type);
    void remove(Entity* ent);
    void hurt(Entity* ent, uint8_t damage);
};

#endif
