#include <stdlib.h>
#include <string.h>
#include "frogboy.h"
#include "particle.h"
#include "camera.h"
#include "sprites_bitmap.h"

namespace {
    uint8_t particleSpawnIndex;
}

Particle Particle::data[PARTICLE_COUNT];

void Particle::initSystem() {
    memset(data, 0, sizeof(data));
    particleSpawnIndex = 0;
}

void Particle::add(int16_t x, int16_t y, int8_t xspd, int8_t yspd, uint8_t tile, uint8_t time) {
    Particle* particle = &data[particleSpawnIndex];
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

void Particle::addStar(int16_t x, int16_t y) {
    int8_t xspd = (frogboy::getRandom(0, 1) == 0 ? -1 : 1) * frogboy::getRandom(4, 8);
    Particle::add(x - 4 * 16, y - 4 * 16, xspd, -12, 0x42, 18);
}

void Particle::updateAll() {
    Particle* particle = &data[0];
    Particle* particleEnd = particle + PARTICLE_COUNT;
    for(; particle != particleEnd; ++particle) {
        if(particle->time > 0) {
            particle->time--;
            particle->x += particle->xspd;
            particle->y += particle->yspd;
        }
    }
}

void Particle::drawAll() {
    Particle* particle = &data[0];
    Particle* particleEnd = particle + PARTICLE_COUNT;
    for(; particle != particleEnd; ++particle) {
        if(particle->time > 0) {
            frogboy::drawTile(particle->x / 16 - camera.x, particle->y / 16 - camera.y, spritesBitmap, particle->tile, 1, false, false);
        }
    }
}