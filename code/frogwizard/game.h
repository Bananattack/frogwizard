#ifndef GAME_H
#define GAME_H

enum GameMode {
    GAME_MODE_TITLE,
    GAME_MODE_ACTIVE,
    GAME_MODE_PAUSE,
};

extern GameMode gameMode;

void gameInit();
void gameDraw();
void gameUpdate();

#endif