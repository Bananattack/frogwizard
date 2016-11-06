#include <stdlib.h>
#include <string.h>
#include "particle.h"
#include "map.h"
#include "arduboyx.h"
#include "sprites_bitmap.h"

Particle particles[PARTICLE_COUNT];
uint8_t particleSpawnIndex;

void particleInitSystem() {
    memset(particles, 0, sizeof(particles));
    particleSpawnIndex = 0;
}

void particleAdd(int16_t x, int16_t y, int8_t xspd, int8_t yspd, uint8_t tile, uint8_t time) {
    Particle* particle = &particles[particleSpawnIndex];
    particle->x = x;
    particle->y = y;
    particle->xspd = xspd;
    particle->yspd = yspd;
    particle->tile = tile;
    particle->time = time;

    particleSpawnIndex++;
    if(particleSpawnIndex >= PARTICLE_COUNT) {
        particleSpawnIndex = 0;
    }
}

void particleStarAdd(int16_t x, int16_t y) {
    int8_t xspd = (random() % 2 ? -1 : 1) * random(4, 8);
    particleAdd(x - 4 * 16, y - 4 * 16, xspd, -12, 0x42, 18);
}

void particleUpdateAll() {
    for(uint8_t i = 0; i != PARTICLE_COUNT; ++i) {
        Particle* particle = &particles[i];
        if(particle->time > 0) {
            particle->time--;
            particle->x += particle->xspd;
            particle->y += particle->yspd;
        }
    }
}

void particleDrawAll() {
    for(uint8_t i = 0; i != PARTICLE_COUNT; ++i) {
        Particle* particle = &particles[i];
        if(particle->time > 0) {
            arduboy.drawTile(particle->x / 16 - mapCameraX, particle->y / 16 - mapCameraY, spritesBitmap, particle->tile, 1, false, false);
        }
    }
}