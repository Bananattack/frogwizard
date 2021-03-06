#ifdef FROGBOY_HAS_MAIN

#include <stdlib.h>
#include "frogboy.h"
#include "game.h"

extern "C" int main(int argc, char** arv) {
    if(frogboy::init()) {
        gameInit();

        while(frogboy::isActive()) {        
            if(frogboy::waitForFrame()) {
                frogboy::clearScreen();
                gameDraw();
                frogboy::refreshScreen();

                gameUpdate();
            }
        }
    }
    frogboy::destroy();
    return 0;
}
#endif