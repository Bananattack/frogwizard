#include "arduboyx.h"
#include "entity.h"
#include "bullet.h"
#include "critter.h"
#include "player.h"
#include "particle.h"
#include "map.h"

void setup() {
    arduboy.begin(); 
    arduboy.setFrameRate(60);
    arduboy.initRandomSeed();

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

void loop() {
    if(!arduboy.nextFrame()) {
        return;
    }
    
    arduboy.clear();

    //mapCameraX = ents[ENT_OFFSET_PLAYER].x / 16 + 8 - WIDTH / 2; 

    int16_t playerX = ents[ENT_OFFSET_PLAYER].x / 16 + 8;    
    if(playerX < mapCameraX + WIDTH / 2 - CAMERA_BORDER) {
        int16_t distance = mapCameraX + WIDTH / 2 - CAMERA_BORDER - playerX;
        if(distance > CAMERA_MAX_SPEED) {
            distance = CAMERA_MAX_SPEED;
        }
        mapCameraX -= distance;
    }
    if(playerX > mapCameraX + WIDTH / 2 + CAMERA_BORDER) {
        int16_t distance = playerX - (mapCameraX + WIDTH / 2 + CAMERA_BORDER);
        if(distance > CAMERA_MAX_SPEED) {
            distance = CAMERA_MAX_SPEED;
        }
        mapCameraX += distance;
    }

    if(mapCameraX <= 0) {
        mapCameraX = 0;
    }
    if(mapCameraX >= mapGetWidth() * 16 - WIDTH) {
        mapCameraX = mapGetWidth() * 16 - WIDTH;
    }

    mapDraw();
    particleDrawAll();
    critterDrawAll();    
    playerDraw(); 
    bulletDrawAll();
    playerDrawHUD();
    arduboy.display();
    
    particleUpdateAll();
    critterUpdateAll();
    bulletUpdateAll();
    playerUpdate();
}
