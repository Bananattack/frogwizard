#ifndef GAME_H
#define GAME_H

enum DoorType {
    DOOR_TYPE_START,
    DOOR_TYPE_GRASSLAND_HOUSE,
    DOOR_TYPE_HOUSE_GRASSLAND,
    DOOR_TYPE_GRASSLAND_HOUSE2,
    DOOR_TYPE_HOUSE2_GRASSLAND,

    DOOR_TYPE_COUNT,
};

void gameInit();
void gameEnterDoor(DoorType door);
bool gameCheckOnScreen(int16_t x, int16_t y, uint8_t borderX, uint8_t borderY);
void gameDraw();
void gameUpdate();

#endif