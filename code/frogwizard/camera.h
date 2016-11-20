#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>

struct Camera {
    int16_t x, y;

    bool containsPoint(int16_t x, int16_t y, uint8_t borderX, uint8_t borderY) const;
    void clamp(int16_t width, int16_t height);
    void reset(int16_t targetX, int16_t targetY, int16_t width, int16_t height);
    void update(int16_t targetX, int16_t targetY, int16_t width, int16_t height);
};

extern Camera camera;

#endif
