#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"

enum {
    EGG_COUNT = 8,
    EGG_BIT_ARRAY_SIZE = (EGG_COUNT + 7) / 8,
};

struct PlayerStatus {
    uint8_t hp;
    uint8_t maxHP;
    uint8_t maxShotCount;
    uint8_t nextDoor;
    uint8_t lastDoor;
    uint8_t eggs[EGG_BIT_ARRAY_SIZE];
    bool dir;
    bool usedDoor;
};

struct PlayerInstance {
    uint8_t frame;
    uint8_t moveTimer;
    uint8_t shootTimer;
    uint8_t jumpTimer;
    uint8_t hurtTimer;
    uint8_t shotCount;
    uint8_t bufferShootTimer;
    uint8_t bufferJumpTimer;
    uint8_t fallTimer;
    bool landed;
    bool jumpPressed;
    bool shootPressed;
    bool pushing;
};

struct Player {
    PlayerStatus status;
    PlayerInstance instance;

    static void initSystem();
    static void add(int16_t x, int16_t y);

    void update();
    void hurt();
    void draw() const;
    void drawHUD() const;
};

extern Player player;

#endif
