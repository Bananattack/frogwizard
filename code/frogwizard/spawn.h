#ifndef SPAWN_H
#define SPAWN_H

#include <stdint.h>

enum {
    SPAWN_MAX = 64,
    SPAWN_BIT_ARRAY_SIZE = (SPAWN_MAX + 7) / 8,
};

namespace spawn {
    void init();
    void suspend(uint8_t index, int16_t x, int16_t y);
    void check();
}
#endif