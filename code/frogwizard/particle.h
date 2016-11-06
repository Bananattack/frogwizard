#ifndef PARTICLE_H
#define PARTICLE_H

#include <stdint.h>

enum {
    PARTICLE_COUNT = 4,
};

struct Particle {
    int16_t x;
    int16_t y;
    int8_t xspd;
    int8_t yspd;
    uint8_t tile;
    uint8_t time;
};

extern Particle particles[PARTICLE_COUNT];

void particleInitSystem();
void particleAdd(int16_t x, int16_t y, int8_t xspd, int8_t yspd, uint8_t tile, uint8_t time);
void particleStarAdd(int16_t x, int16_t y);
void particleUpdateAll();
void particleDrawAll();

#endif