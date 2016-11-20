#include "frogboy.h"
#include "camera.h"

namespace {
    enum {
        CAMERA_BORDER = 6,
        CAMERA_MAX_SPEED = 4,
    };
}

Camera camera;

bool Camera::containsPoint(int16_t x, int16_t y, uint8_t borderX, uint8_t borderY) const {
    static_cast<void>(y);
    static_cast<void>(borderY);
    return x - this->x < frogboy::SCREEN_WIDTH + borderX
        && x - this->x > -borderX;
        //&& y - this->y < frogboy::SCREEN_HEIGHT + borderY
        //&& y - this->y > -borderY;
}

void Camera::clamp(int16_t width, int16_t height) {
    static_cast<void>(height);
    if(x <= 0) {
        x = 0;
    }
    if(x >= width * 16 - frogboy::SCREEN_WIDTH) {
        x = width * 16 - frogboy::SCREEN_WIDTH;
    }
}

void Camera::reset(int16_t targetX, int16_t targetY, int16_t width, int16_t height) {
    static_cast<void>(targetY);
    x = targetX;
    clamp(width, height);
}

void Camera::update(int16_t targetX, int16_t targetY, int16_t width, int16_t height) {
    static_cast<void>(targetY);
    if(targetX < x + frogboy::SCREEN_WIDTH / 2 - CAMERA_BORDER) {
        int16_t distance = x + frogboy::SCREEN_WIDTH / 2 - CAMERA_BORDER - targetX;
        if(distance > CAMERA_MAX_SPEED) {
            distance = CAMERA_MAX_SPEED;
        }
        x -= distance;
    }
    if(targetX > x + frogboy::SCREEN_WIDTH / 2 + CAMERA_BORDER) {
        int16_t distance = targetX - (x + frogboy::SCREEN_WIDTH / 2 + CAMERA_BORDER);
        if(distance > CAMERA_MAX_SPEED) {
            distance = CAMERA_MAX_SPEED;
        }
        x += distance;
    }
	
    clamp(width, height);
}