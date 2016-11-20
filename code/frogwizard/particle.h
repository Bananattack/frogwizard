#ifndef PARTICLE_H
#define PARTICLE_H

#include <stdint.h>

enum {
    PARTICLE_COUNT = 6,
};

struct Particle {
    int16_t x;
    int16_t y;
    int8_t xspd;
    int8_t yspd;
    uint8_t tile;
    uint8_t time;

    static Particle data[PARTICLE_COUNT];

    static void initSystem();
    static void add(int16_t x, int16_t y, int8_t xspd, int8_t yspd, uint8_t tile, uint8_t time);
    static void addStar(int16_t x, int16_t y);
    static void updateAll();
    static void drawAll();
};

#endif