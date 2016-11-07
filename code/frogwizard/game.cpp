#include "frogboy.h"
#include "entity.h"
#include "bullet.h"
#include "critter.h"
#include "player.h"
#include "particle.h"
#include "map.h"

void gameInit() {
    mapInitSystem();
    entityInitSystem();
    critterInitSystem();
    bulletInitSystem();
    particleInitSystem();

    playerAdd(32 * 16, 32 * 16);
    critterAdd(144 * 16, 16 * 16, CRITTER_TYPE_WALKER);
    player.dir = true;
}

enum {
    CAMERA_BORDER = 8,
    CAMERA_MAX_SPEED = 4,
};

void gameDraw() {
    mapDraw();
    particleDrawAll();
    critterDrawAll();    
    playerDraw(); 
    bulletDrawAll();
    playerDrawHUD();
}

void gameUpdate() {    
    particleUpdateAll();
    critterUpdateAll();
    bulletUpdateAll();
    playerUpdate();

    int16_t playerX = ents[ENT_OFFSET_PLAYER].x / 16 + 8;
    if(playerX < mapCameraX + frogboy::SCREEN_WIDTH / 2 - CAMERA_BORDER) {
        int16_t distance = mapCameraX + frogboy::SCREEN_WIDTH / 2 - CAMERA_BORDER - playerX;
        if(distance > CAMERA_MAX_SPEED) {
            distance = CAMERA_MAX_SPEED;
        }
        mapCameraX -= distance;
    }
    if(playerX > mapCameraX + frogboy::SCREEN_WIDTH / 2 + CAMERA_BORDER) {
        int16_t distance = playerX - (mapCameraX + frogboy::SCREEN_WIDTH / 2 + CAMERA_BORDER);
        if(distance > CAMERA_MAX_SPEED) {
            distance = CAMERA_MAX_SPEED;
        }
        mapCameraX += distance;
    }
	
    if(mapCameraX <= 0) {
        mapCameraX = 0;
    }
    if(mapCameraX >= mapGetWidth() * 16 - frogboy::SCREEN_WIDTH) {
        mapCameraX = mapGetWidth() * 16 - frogboy::SCREEN_WIDTH;
    }

    if(frogboy::isPressed(frogboy::BUTTON_RESET)) {
        gameInit();
    }
}