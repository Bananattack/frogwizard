#include "frogboy.h"
#include "game.h"

void setup() {
    frogboy::init();
    gameInit();
}

void loop() {
    if(frogboy::waitForFrame()) {    
        frogboy::clearScreen();
        gameDraw();
        frogboy::refreshScreen();

        gameUpdate();
    }
}