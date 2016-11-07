#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"

struct Player {
    uint8_t hp;
    uint8_t maxHP;    
    uint8_t frame;
    uint8_t timer;    
    uint8_t shootTimer;    
    uint8_t jumpTimer;
    uint8_t hurtTimer;
    uint8_t shotCount;
    uint8_t maxShotCount;
    uint8_t bufferShootTimer;
    uint8_t bufferJumpTimer;
    uint8_t fallTimer;
    bool dir;
    bool landed;
    bool jumpPressed;
    bool shootPressed;
};

extern Player player;

void playerAdd(int16_t x, int16_t y);
void playerUpdate();
void playerHurt();
void playerDraw();
void playerDrawHUD();

#endif
