#include "frogboy.h"
#include "entity.h"
#include "bullet.h"
#include "critter.h"
#include "player.h"
#include "particle.h"
#include "sprites_bitmap.h"
#include "map.h"

bool pausePressed = false;
bool gamePaused = false;

const char pauseMessage[] FROGBOY_ROM_DATA = "PAUSED";

void gameInit() {
    mapInitSystem();
    entityInitSystem();
    critterInitSystem();
    bulletInitSystem();
    particleInitSystem();

    playerAdd(32 * 16, 32 * 16);
    critterAdd(160 * 16, 16 * 16, CRITTER_TYPE_DOOR);
    critterAdd(144 * 16, 16 * 16, CRITTER_TYPE_WALKER);
    critterAdd(384 * 16, 16 * 16, CRITTER_TYPE_WALKER);
    player.dir = true;
    pausePressed = true;
}

enum {
    CAMERA_BORDER = 8,
    CAMERA_MAX_SPEED = 4,
};

void gameDraw() {
    mapDraw();
    critterDrawAll();
    playerDraw(); 
    bulletDrawAll();   
    particleDrawAll();
    if(gamePaused) {
        for(uint8_t i = 0; i != sizeof(pauseMessage) + 1; ++i) {
            frogboy::drawTile(32 + i * 8, 24, spritesBitmap, 0x4A, 0, false, false);
        }
        frogboy::printRomString(40, 24, spritesBitmap, pauseMessage, 1);
    }

    playerDrawHUD();
}
        
void gameUpdatePaused() {

}

void gameUpdate() {    
    if(frogboy::isPressed(frogboy::BUTTON_PAUSE)) {
        if(!pausePressed) {
            pausePressed = true;
            gamePaused = !gamePaused;
        }
    } else {
        pausePressed = false;
    }

    if(gamePaused) {
        gameUpdatePaused();
        return;
    }

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