#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

enum EntityCtrlFlags {
    ENT_CTRL_FLAG_IGNORE_OBS = 1 << 0,
    ENT_CTRL_FLAG_IGNORE_SLOPES = 1 << 1,
    ENT_CTRL_FLAG_ACTIVE = 1 << 7,
};

enum EntityDrawFlags {
    ENT_DRAW_FLAG_FLASH = 1 << 0,
    ENT_DRAW_FLAG_HFLIP = 1 << 1,
    ENT_DRAW_FLAG_VFLIP = 1 << 2,
    ENT_DRAW_FLAG_HIDDEN = 1 << 3,
};

enum EntityStatus {
    ENT_STATUS_HIT_OBS_X = 1 << 0,
    ENT_STATUS_HIT_OBS_Y = 1 << 1,
    ENT_STATUS_HIT_OBS = ENT_STATUS_HIT_OBS_X | ENT_STATUS_HIT_OBS_Y,    
    ENT_STATUS_UPHILL = 1 << 2
};

enum {
    ENT_COUNT_PLAYER = 1,
    ENT_COUNT_CRITTER = 4,
    ENT_COUNT_BULLET = 8,

    ENT_OFFSET_PLAYER = 0,
    ENT_OFFSET_CRITTER = ENT_OFFSET_PLAYER + ENT_COUNT_PLAYER,
    ENT_OFFSET_BULLET = ENT_OFFSET_CRITTER + ENT_COUNT_CRITTER,

    ENT_COUNT = ENT_COUNT_PLAYER + ENT_COUNT_CRITTER + ENT_COUNT_BULLET
};

struct Entity {
    int16_t x;
    int16_t y;
    int16_t xspd;
    int16_t yspd;
    uint8_t controlFlags;
    uint8_t drawFlags;
    uint8_t status;
    uint8_t sprite;
    uint8_t hitbox;
};

enum DamageStatus {
    DAMAGE_IGNORED,
    DAMAGE_REJECTED,
    DAMAGE_ACCEPTED,
};

extern Entity ents[ENT_COUNT];

void entityInitSystem();
uint8_t entityAdd(int16_t x, int16_t y, uint8_t offset, uint8_t count);
void entityRemove(uint8_t entityIndex);
bool entityDetectFloor(uint8_t entityIndex);
void entityUpdate(uint8_t entityIndex);
void entityDraw(uint8_t entityIndex);
bool entityCollide(uint8_t entityIndexA, int8_t borderA, uint8_t entityIndexB, int8_t borderB);
bool entityOnScreen(uint8_t entityIndex);

#endif
