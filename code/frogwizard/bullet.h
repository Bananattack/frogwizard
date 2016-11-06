#ifndef BULLET_H
#define BULLET_H

#include "entity.h"

enum BulletType {
    BULLET_TYPE_FIREBALL,
    BULLET_TYPE_EXPLOSION,
    BULLET_TYPE_COUNT,
};

struct Bullet {
    uint8_t type;
    uint8_t flags;
    bool dir;
    uint8_t timer;
};

void bulletInitSystem();
uint8_t bulletAdd(int16_t x, int16_t y, bool dir, BulletType type);
void bulletRemove(uint8_t bulletIndex);
void bulletUpdateAll();
void bulletDrawAll();

#endif
