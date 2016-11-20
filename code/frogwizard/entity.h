#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

enum EntityCtrlFlags {
    // Entity should move freely through walls and obstructions.
    ENT_CTRL_FLAG_IGNORE_OBS = 1 << 0,
    // Entity should move without regard for slope obstructions
    ENT_CTRL_FLAG_IGNORE_SLOPES = 1 << 1,
    // Allow bullet collisions. (TODO: Maybe move this into collision category instead)
    ENT_CTRL_FLAG_BULLET_TARGET = 1 << 3,
    // Entity is in use.
    ENT_CTRL_FLAG_ACTIVE = 1 << 7,
};

enum EntityDrawFlags {
    // Invert colors.
    ENT_DRAW_FLAG_FLASH = 1 << 0,
    // Horizontal flip.
    ENT_DRAW_FLAG_HFLIP = 1 << 1,
    // Vertical flip.
    ENT_DRAW_FLAG_VFLIP = 1 << 2,
    // Treat sprite as text ID instead.
    ENT_DRAW_FLAG_TEXT = 1 << 4,
    // Omit from rendering.
    ENT_DRAW_FLAG_HIDDEN = 1 << 7,
};

enum EntityStatus {
    ENT_STATUS_HIT_OBS_X = 1 << 0,
    ENT_STATUS_HIT_OBS_Y = 1 << 1,
    ENT_STATUS_HIT_OBS = ENT_STATUS_HIT_OBS_X | ENT_STATUS_HIT_OBS_Y,    
    ENT_STATUS_UPHILL = 1 << 2,
    ENT_STATUS_DOWNHILL = 1 << 3,
};

enum CollisionCategory {
    COLLISION_CATEGORY_CRITTER,
    COLLISION_CATEGORY_BLOCK,
    COLLISION_CATEGORY_ZONE,

    COLLISION_CATEGORY_COUNT,
};

enum CollisionMask {
    COLLISION_MASK_CRITTER = 1 << COLLISION_CATEGORY_CRITTER,
    COLLISION_MASK_BLOCK = 1 << COLLISION_CATEGORY_BLOCK,
};

enum {
    ENT_COUNT_PLAYER = 1,
    ENT_COUNT_CRITTER = 6,
    ENT_COUNT_BULLET = 6,

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
    uint8_t collisionCategory;

    static Entity data[ENT_COUNT];

    static void initSystem();
    static Entity* add(int16_t x, int16_t y, uint8_t offset, uint8_t count);

    void remove();
    bool getEntPixelObs(int16_t x, int16_t y);
    bool getAnyPixelObs(int16_t x, int16_t y);
    bool detectFloor(int8_t hx, int8_t hy, int8_t hw, int8_t hh, bool ignoreEnts);
    bool detectFloor();
    void update();
    void draw();
    bool collide(int8_t thisBorder, Entity* other, int8_t otherBorder);
};

#endif
