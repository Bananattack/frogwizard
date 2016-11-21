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
#include "camera.h"
#include "door.h"
#include "spawn.h"
#include "text.h"
#include "save.h"

bool pausePressed;
bool resetPressed;
GameMode gameMode;
bool saveFound;
bool titleCursorPressed;
uint8_t titleCursor;

uint8_t wipeProgress;
const uint8_t wipeMasks[] FROGBOY_ROM_DATA = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x51, 0x55, 0xD5, 0xDD, 0xDF, 0xFF};

typedef void (*GameModeHandler)();
extern const GameModeHandler gameModeUpdateHandlers[] FROGBOY_ROM_DATA;
extern const GameModeHandler gameModeDrawHandlers[] FROGBOY_ROM_DATA;

void gameInit() {
    gameMode = GAME_MODE_TITLE;
    resetPressed = pausePressed = titleCursorPressed = true;
    saveFound = save::exists();
    titleCursor = saveFound ? 1 : 0;
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

void gameEnterDoor(DoorType door) {
    frogboy::playTone(390, 10);
    Entity::initSystem();
    Critter::initSystem();
    Bullet::initSystem();
    Particle::initSystem();
    spawn::init();
    
    int16_t x = 0;
    int16_t y = 0;
    door::read(door, x, y, map.currentIndex);
    player.add(x, y);

    wipeProgress = 0;

    camera.reset(camera.x = Entity::data[ENT_OFFSET_PLAYER].x / 16 + 8 - frogboy::SCREEN_WIDTH / 2, 0, map.getWidth(), map.getHeight());
    Critter::updateAll();
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
    text::printCenter(64, 8, TEXT_TYPE_TITLE, 1);
    text::print(40, 24, TEXT_TYPE_NEW_GAME, 1);
    if(saveFound) {
        text::print(40, 32, TEXT_TYPE_CONTINUE, 1);
    }
    sprite::draw(8, 24, player.instance.moveTimer % 32 < 16 ? SPRITE_TYPE_PLAYER_1 : SPRITE_TYPE_PLAYER_2, SPRITE_FLAG_HFLIP);
    frogboy::drawTile(31, 24 + titleCursor * 8, spritesBitmap, 0x40, 1, false, false);
    text::printCenter(64, 48, TEXT_TYPE_AUTHOR, 1);
}

void gameModeTitleUpdate() {
    player.instance.moveTimer++;
    if(frogboy::anyPressed(frogboy::BUTTON_MASK_UP | frogboy::BUTTON_MASK_DOWN)) {
        if(!titleCursorPressed && saveFound) {
            titleCursor ^= 1;
            titleCursorPressed = true;
            frogboy::playTone(3000, 50);
        }
    } else {
        titleCursorPressed = false;
    }

    if(!resetPressed && frogboy::isPressed(frogboy::BUTTON_JUMP)) {
        gameMode = GAME_MODE_ACTIVE;

        // Initialize state before loading the player's save.
        player.initSystem();

        if(titleCursor == 1) {
            // If we fail to load, reset to new game anyways.
            if(!save::load()) {
                player.initSystem();
            }
        }

        Map::initSystem();
        gameEnterDoor(static_cast<DoorType>(player.status.lastDoor));
        player.instance.shootPressed = player.instance.jumpPressed = true;
        player.update();
    }
}

void gameModeActiveDraw() {
    map.draw(camera);
    Critter::drawAll();
    player.draw(); 
    Bullet::drawAll();   
    Particle::drawAll();

    uint8_t mask = frogboy::readRom<uint8_t>(&wipeMasks[wipeProgress >> 2]);
    uint8_t* buffer = frogboy::getScreenBuffer();
    for(uint16_t i = 0; i != frogboy::SCREEN_WIDTH * (frogboy::SCREEN_HEIGHT / 8); ++i) {
        *buffer = *buffer & mask;
        buffer++;
    }

    player.drawHUD();
}

void gameModeActiveUpdate() {
    gameCheckPauseToggle(GAME_MODE_PAUSE);

    if(wipeProgress < 48) {
        wipeProgress++;
        if(wipeProgress == 20 && player.status.usedDoor) {
            frogboy::playTone(300, 4);
        }
    }
    if(wipeProgress >= 36) {
        spawn::check();
        Particle::updateAll();
        Critter::updateAll();
        Bullet::updateAll();
        player.update();
        camera.update(Entity::data[ENT_OFFSET_PLAYER].x / 16 + 8, 0, map.getWidth(), map.getHeight());
    }

    if(player.status.nextDoor != 0xFF) {
        gameEnterDoor(static_cast<DoorType>(player.status.nextDoor));
        player.status.lastDoor = player.status.nextDoor;
        player.status.nextDoor = 0xFF;
        save::save();
    }
}

void gameModePauseDraw() {
    gameModeActiveDraw();
    for(uint8_t i = 0; i != text::length(TEXT_TYPE_PAUSED) + 1; ++i) {
        frogboy::drawTile(32 + i * 8, 24, spritesBitmap, 0x4A, 0, false, false);
    }
    text::printCenter(64, 24, TEXT_TYPE_PAUSED, 1);
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