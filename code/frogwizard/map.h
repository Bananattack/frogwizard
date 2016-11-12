#ifndef MAP_H
#define MAP_H

#include "frogboy.h"

enum MapType {
    MAP_TYPE_GRASSLAND,
    MAP_TYPE_HOUSE,
    MAP_TYPE_HOUSE2,
    MAP_TYPE_HOUSE3,

    MAP_TYPE_COUNT,
};

void mapInitSystem();
uint8_t mapGetPixelAttribute(int16_t x, int16_t y);
bool mapGetPixelAttributeObs(uint8_t attr, int16_t x, int16_t y);
bool mapGetPixelObs(int16_t x, int16_t y);
void mapDraw();
uint8_t mapGetWidth();

extern uint8_t mapCurrentIndex;
extern int16_t mapCameraX, mapCameraY;

#endif