#include <string.h>
#include "frogboy.h"
#include "game.h"
#include "entity.h"
#include "bullet.h"
#include "critter.h"
#include "player.h"
#include "particle.h"
#include "sprite.h"
#include "sprites_bitmap.h"
#include "map.h"

enum GameMode {
    GAME_MODE_TITLE,
    GAME_MODE_ACTIVE,
    GAME_MODE_PAUSE,
};

bool pausePressed;
bool resetPressed;
GameMode gameMode;

const char titleText[] FROGBOY_ROM_DATA = "F R O G  E G G";
const char pressStartText[] FROGBOY_ROM_DATA = "PRESS START";
const char authorText[] FROGBOY_ROM_DATA = "BY EGGBOYCOLOR";
const char pauseText[] FROGBOY_ROM_DATA = "PAUSED";

uint8_t wipeProgress;
const uint8_t wipeMasks[] FROGBOY_ROM_DATA = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x51, 0x55, 0xD5, 0xDD, 0xDF, 0xFF};

const uint8_t doorData[DOOR_TYPE_COUNT * 3] FROGBOY_ROM_DATA = {
    2, 2, MAP_TYPE_GRASSLAND,
    2, 2, MAP_TYPE_HOUSE,
    10, 1, MAP_TYPE_GRASSLAND,
    5, 2, MAP_TYPE_HOUSE2,
    40, 1, MAP_TYPE_GRASSLAND,
    2, 2, MAP_TYPE_HOUSE3,
    67, 2, MAP_TYPE_GRASSLAND,
};

typedef void (*GameModeHandler)();
extern const GameModeHandler gameModeUpdateHandlers[] FROGBOY_ROM_DATA;
extern const GameModeHandler gameModeDrawHandlers[] FROGBOY_ROM_DATA;

void gameInit() {
    gameMode = GAME_MODE_TITLE;
    resetPressed = pausePressed = true;
    //frogboy::playMusic(musicScore);
}

void gameDraw() {
    frogboy::readRom<GameModeHandler>(&gameModeDrawHandlers[gameMode])();
}

void gameUpdate() {
    if(frogboy::isPressed(frogboy::BUTTON_RESET)) {
        if(!resetPressed) {
            gameInit();
            return;
        }
    } else {
        if(!frogboy::anyPressed(frogboy::BUTTON_MASK_PAUSE | frogboy::BUTTON_MASK_JUMP | frogboy::BUTTON_MASK_SHOOT)) {
            resetPressed = false;
        }
    }

    frogboy::readRom<GameModeHandler>(&gameModeUpdateHandlers[gameMode])();
}

enum {
    CAMERA_BORDER = 6,
    CAMERA_MAX_SPEED = 4,
};
 
bool gameCheckOnScreen(int16_t x, int16_t y, uint8_t borderX, uint8_t borderY) {
    static_cast<void>(y);
    static_cast<void>(borderY);
    return x - mapCameraX < frogboy::SCREEN_WIDTH + borderX
        && x - mapCameraX > -borderX;
        //&& y - mapCameraY < frogboy::SCREEN_HEIGHT + borderY
        //&& y - mapCameraY > -borderY;
}

void gameUpdateCamera() {
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
}

void gameEnterDoor(DoorType door) {
    const uint8_t* doorPtr = &doorData[door * 3];

    entityInitSystem();
    critterInitSystem();
    bulletInitSystem();
    particleInitSystem();
    
    int16_t x = frogboy::readRom<uint8_t>(doorPtr++) * 256;
    int16_t y = frogboy::readRom<uint8_t>(doorPtr++) * 256;
    mapCurrentIndex = frogboy::readRom<uint8_t>(doorPtr++);
    playerAdd(x, y);

    wipeProgress = 0;

    mapCameraX = ents[ENT_OFFSET_PLAYER].x / 16 + 8 - frogboy::SCREEN_WIDTH / 2;
    gameUpdateCamera();
    critterUpdateAll();
}

void gameCheckPauseToggle(GameMode nextMode) {
    if(frogboy::isPressed(frogboy::BUTTON_PAUSE)) {
        if(!pausePressed) {
            pausePressed = true;
            gameMode = nextMode;
            frogboy::playTone(1500, 50);
        }
    } else {
        pausePressed = false;
    }
}

void gameModeTitleDraw() {
    frogboy::printRomString(64 - sizeof(titleText) / 2 * 8, 8, spritesBitmap, titleText, 1);
    frogboy::printRomString(128 - sizeof(pressStartText) * 8, 24, spritesBitmap, pressStartText, 1);
    spriteDraw(8, 20, player.timer % 32 < 16 ? SPRITE_TYPE_PLAYER_1 : SPRITE_TYPE_PLAYER_2, SPRITE_FLAG_HFLIP);
    frogboy::printRomString(64 - sizeof(authorText) / 2 * 8, 48, spritesBitmap, authorText, 1);
}

void gameModeTitleUpdate() {
    player.timer++;
    if(!resetPressed && frogboy::anyPressed(frogboy::BUTTON_MASK_PAUSE | frogboy::BUTTON_MASK_JUMP | frogboy::BUTTON_MASK_SHOOT)) {
        gameMode = GAME_MODE_ACTIVE;

        playerInitSystem();
        mapInitSystem();
        gameEnterDoor(DOOR_TYPE_START);
        player.shootPressed = player.jumpPressed = true;
        playerUpdate();
    }
}

void gameModeActiveDraw() {
    mapDraw();
    critterDrawAll();
    playerDraw(); 
    bulletDrawAll();   
    particleDrawAll();

    uint8_t mask = frogboy::readRom<uint8_t>(&wipeMasks[wipeProgress >> 2]);
    uint8_t* buffer = frogboy::getScreenBuffer();
    for(uint16_t i = 0; i != frogboy::SCREEN_WIDTH * (frogboy::SCREEN_HEIGHT / 8); ++i) {
        *buffer = *buffer & mask;
        buffer++;
    }

    playerDrawHUD();
}

void gameModeActiveUpdate() {
    gameCheckPauseToggle(GAME_MODE_PAUSE);

    if(wipeProgress < 48) {
        wipeProgress++;
        if(wipeProgress == 20 && playerStatus.usedDoor) {
            frogboy::playTone(300, 4);
        }
    }
    if(wipeProgress >= 36) {
        particleUpdateAll();
        critterUpdateAll();
        bulletUpdateAll();
        playerUpdate();
        gameUpdateCamera();
    }

    if(playerStatus.nextMap != 0xFF) {
        gameEnterDoor(static_cast<DoorType>(playerStatus.nextMap));
        playerStatus.nextMap = 0xFF;
    }
}

void gameModePauseDraw() {
    gameModeActiveDraw();
    for(uint8_t i = 0; i != sizeof(pauseText) + 1; ++i) {
        frogboy::drawTile(32 + i * 8, 24, spritesBitmap, 0x4A, 0, false, false);
    }
    frogboy::printRomString(40, 24, spritesBitmap, pauseText, 1);
}

void gameModePauseUpdate() {
    gameCheckPauseToggle(GAME_MODE_ACTIVE);
}

const GameModeHandler gameModeDrawHandlers[] = {
    gameModeTitleDraw,
    gameModeActiveDraw,
    gameModePauseDraw,
};

const GameModeHandler gameModeUpdateHandlers[] = {
    gameModeTitleUpdate,
    gameModeActiveUpdate,
    gameModePauseUpdate,
};