#ifndef BULLET_H
#define BULLET_H

#include "entity.h"

enum BulletType {
    BULLET_TYPE_FIREBALL,
    BULLET_TYPE_EXPLOSION,
    BULLET_TYPE_COUNT,
};

enum BulletFlags {
    BULLET_FLAG_PLAYER_BULLET = 0x01,
};

struct Bullet {
    uint8_t type;
    uint8_t flags;
    uint8_t timer;
    bool dir;
};

void bulletInitSystem();
Bullet* bulletAdd(int16_t x, int16_t y, bool dir, BulletType type);
void bulletRemove(Entity* entity, Bullet* bullet);
void bulletUpdateAll();
void bulletDrawAll();

#endif
