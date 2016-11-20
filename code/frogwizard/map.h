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

struct Camera;

struct Map {
    uint8_t currentIndex;

    static void initSystem();

    uint8_t getPixelAttribute(int16_t x, int16_t y) const;
    bool getPixelAttributeObs(uint8_t attr, int16_t x, int16_t y) const;
    bool getPixelObs(int16_t x, int16_t y) const;
    uint8_t getWidth() const;
    uint8_t getHeight() const;
    void draw(const Camera& camera) const;
};

extern Map map;

#endif