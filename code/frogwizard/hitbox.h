#ifndef HITBOX_H
#define HITBOX_H

#include <stdint.h>

enum HitboxType {
    HITBOX_TYPE_EMPTY,
    HITBOX_TYPE_1x1,
    HITBOX_TYPE_16x16,
    HITBOX_TYPE_8x8,
    HITBOX_TYPE_BULLET_16x16,
    HITBOX_TYPE_HUMAN_16x16,
    HITBOX_TYPE_BLOCK,
    HITBOX_TYPE_PUSH_DETECTION,

    HITBOX_TYPE_COUNT,
};

namespace hitbox {
    bool collide(int16_t xa, int16_t ya, HitboxType hitboxA, int8_t borderA, int16_t xb, int16_t yb, HitboxType hitboxB, int8_t borderB);
    void read(HitboxType hitbox, int8_t* x, int8_t* y, int8_t* w, int8_t* h);
};

#endif
