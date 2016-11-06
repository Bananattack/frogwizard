#ifndef HITBOX_H
#define HITBOX_H

#include <avr/pgmspace.h>
#include <stdint.h>

enum HitboxType {
    HITBOX_TYPE_NONE,
    HITBOX_TYPE_16x16,
    HITBOX_TYPE_8x8,
    HITBOX_TYPE_BULLET_16x16,
    HITBOX_TYPE_HUMAN_16x16,
    HITBOX_TYPE_COUNT,
};

bool hitboxCollide(int16_t xa, int16_t ya, HitboxType hitboxA, int16_t xb, int16_t yb, HitboxType hitboxB);
void hitboxGetData(HitboxType hitbox, int8_t* x, int8_t* y, int8_t* w, int8_t* h);

#endif
