#include "map.h"
#include "sprites_bitmap.h"

uint8_t mapCurrentIndex;
int16_t mapCameraX, mapCameraY;

enum {
    BLOCK_ATTR_OBS_TYPE_EMPTY = 0x00,
    BLOCK_ATTR_OBS_TYPE_SOLID = 0x01,
    BLOCK_ATTR_OBS_TYPE_SLOPE_CEILING_L = 0x02, // slope ceiling, upper left corner is obstructed
    BLOCK_ATTR_OBS_TYPE_SLOPE_CEILING_R = 0x03, // slope ceiling, upper right corner is obstructed
    BLOCK_ATTR_OBS_TYPE_SLOPE_FLOOR_L = 0x04, // slope floor, lower left corner is obstructed
    BLOCK_ATTR_OBS_TYPE_SLOPE_FLOOR_R = 0x05, // slope floor, lower right corner is obstructed

    BLOCK_ATTR_TYPE_MASK = 0x0F,

    BLOCK_ATTR_HFLIP = 0x40,
    BLOCK_ATTR_VFLIP = 0x80,    
};

// The series of 2x2 arrangements of tiles.
const uint8_t blockTiles[] FROGBOY_ROM_DATA = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x45, 0x45, // brick 1/2 floor
    0x45, 0x45, 0x00, 0x00, // brick 1/2 ceiling
    0x45, 0x45, 0x45, 0x45, // brick wall
    0x00, 0x49, 0x49, 0x59, // slope floor / (dirt)
    0x49, 0x00, 0x59, 0x49, // slope floor \ (dirt)
    0x59, 0x59, 0x59, 0x59, // solid dirt
};

// The attributes of each block.
const uint8_t blockAttributes[] FROGBOY_ROM_DATA = {
    0, 0, 0, 0,
    0, 0, BLOCK_ATTR_OBS_TYPE_SOLID, BLOCK_ATTR_OBS_TYPE_SOLID, 
    BLOCK_ATTR_OBS_TYPE_SOLID, BLOCK_ATTR_OBS_TYPE_SOLID, 0, 0, 
    BLOCK_ATTR_OBS_TYPE_SOLID, BLOCK_ATTR_OBS_TYPE_SOLID, BLOCK_ATTR_OBS_TYPE_SOLID, BLOCK_ATTR_OBS_TYPE_SOLID, 
    0, BLOCK_ATTR_OBS_TYPE_SLOPE_FLOOR_R, BLOCK_ATTR_OBS_TYPE_SLOPE_FLOOR_R, BLOCK_ATTR_OBS_TYPE_SOLID, 
    BLOCK_ATTR_OBS_TYPE_SLOPE_FLOOR_L | BLOCK_ATTR_HFLIP, 0, BLOCK_ATTR_OBS_TYPE_SOLID, BLOCK_ATTR_OBS_TYPE_SLOPE_FLOOR_L | BLOCK_ATTR_HFLIP,
    BLOCK_ATTR_OBS_TYPE_SOLID, BLOCK_ATTR_OBS_TYPE_SOLID, BLOCK_ATTR_OBS_TYPE_SOLID, BLOCK_ATTR_OBS_TYPE_SOLID, 
};

// The series of unique 1x4 arrangements of blocks
const uint8_t columnBlocks[] FROGBOY_ROM_DATA = {
    0, 0, 0, 0, // 0: empty
    0, 0, 3, 6, // 1: brick floor 2 on dirt floor 1 
    0, 3, 3, 6, // 2: brick floor 3 on dirt floor 1
    0, 3, 6, 6, // 3: brick floor 3 on dirt floor 2
    0, 0, 0, 4, // 4: dirt floor / 0 to 1    
    0, 0, 4, 6, // 5: dirt floor / 1 to 2
    0, 0, 6, 6, // 6: dirt floor 2
    0, 0, 5, 6, // 7: dirt floor \ 1 to 2
    0, 0, 0, 6, // 8: dirt floor 1
    0, 4, 6, 6, // 9: dirt floor / 2 to 3
    0, 6, 6, 6, // 10: dirt floor 3
    0, 5, 6, 6, // 11: dirt floor \ 2 to 3
    3, 3, 3, 3, // 12: full wall
    3, 0, 0, 3, // 13: brick floor 1, brick ceiling 1
    2, 0, 0, 1, // 14: brick floor 0.5, brick ceiling 0.5
    2, 0, 2, 1, // 15: brick floor 0.5, brick mid 0.5 @ 1.5, brick ceiling 0.5
    2, 1, 0, 1, // 16: brick floor 0.5, brick mid 0.5 @ 2.0, brick ceiling 0.5
    2, 1, 3, 3, // 17: brick floor 2.5, brick ceiling 0.5
    3, 3, 2, 1, // 18: brick floor 0.5, brick ceiling 2.5
    2, 0, 1, 3, // 19: brick floor 1.5, brick ceiling 0.5
    2, 1, 2, 1, // 20: brick floor 0.5, brick mid 1 @ 1.5, brick ceiling 2
};

const uint8_t grasslandMap[] FROGBOY_ROM_DATA = {
    8, 8, 8, 8, 8, 8, 8, 5, 6, 6, 6, 6, 3, 6, 7, 8, 8, 8, 8, 8, 8, 8, 5, 6, 6, 6, 6, 6, 6, 7, 8, 8, 5, 9, 10, 10, 10, 10, 11, 6, 6, 6, 7, 8, 8, 8, 8, 8, 8, 1, 8, 8, 2, 8, 1, 8, 1, 8, 2, 2, 2, 8, 1, 8, 8, 8, 8, 1, 8, 8, 2, 2, 2, 8, 8, 1, 8, 8, 8, 8, 8, 8, 5, 6, 7, 8, 5, 6, 6, 7, 8, 8, 8, 8, 8,
};

const uint8_t houseMap[] FROGBOY_ROM_DATA = {
    0, 12, 13, 13, 13, 13, 12, 0, 
};

const uint8_t houseMap3[] FROGBOY_ROM_DATA = {
    12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 14, 14, 15, 15, 14, 14, 15, 15, 17, 17, 16, 16, 16, 16, 16, 16, 16, 16, 16, 20, 15, 15, 15, 15, 14, 14, 15, 15, 18, 18, 14, 14, 14, 14, 14, 19, 19, 19, 12 
};

// Table of pointers to each map.
// Every map is comprised of a big sequence of column references.
const uint8_t* const mapAddresses[static_cast<uint8_t>(MAP_TYPE_COUNT)] FROGBOY_ROM_DATA = {
    grasslandMap,
    houseMap,
    houseMap,
    houseMap3,
};

// The width of each map. Can be up to 256 tiles wide
const uint8_t mapWidths[static_cast<uint8_t>(MAP_TYPE_COUNT)] FROGBOY_ROM_DATA = {
    sizeof(grasslandMap),
    sizeof(houseMap),
    sizeof(houseMap),
    sizeof(houseMap3),
};

// spawn data: consists of x, y, type, flag

void mapInitSystem() {
    mapCurrentIndex = MAP_TYPE_GRASSLAND;
    mapCameraX = 0;
    mapCameraY = 0;
}

uint8_t mapGetPixelAttribute(int16_t x, int16_t y) {
    int16_t mapTileWidth = static_cast<int16_t>(frogboy::readRom<uint8_t>(mapWidths + mapCurrentIndex));
    int16_t mapPixelWidth = mapTileWidth * 16;

    if(x < 0 || x >= mapPixelWidth) {
        return BLOCK_ATTR_OBS_TYPE_SOLID;
    }

    if(y >= 64) {
        return BLOCK_ATTR_OBS_TYPE_EMPTY;
    }
    if(y < 0) {
        y = 0;
    }

    uint8_t col = static_cast<uint8_t>(x / 16);
    uint8_t row = static_cast<uint8_t>(y / 16);

    const uint8_t* mapPtr = frogboy::readRom<const uint8_t*>(mapAddresses + mapCurrentIndex);
    const uint8_t* columnPtr = columnBlocks + static_cast<uint16_t>(frogboy::readRom<uint8_t>(mapPtr + col)) * 4;
    const uint8_t* blockAttrPtr = blockAttributes + static_cast<uint16_t>(frogboy::readRom<uint8_t>(columnPtr + row)) * 4;
    return frogboy::readRom<uint8_t>(blockAttrPtr + (x % 16 < 8 ? 0 : 1) + (y % 16 < 8 ? 0 : 2));
}

bool mapGetPixelAttributeObs(uint8_t attr, int16_t x, int16_t y) {
    x %= 8;
    y %= 8;

    switch(attr & BLOCK_ATTR_TYPE_MASK) {
        case BLOCK_ATTR_OBS_TYPE_SOLID: return true;
        case BLOCK_ATTR_OBS_TYPE_SLOPE_CEILING_L: return y < 7 - x;
        case BLOCK_ATTR_OBS_TYPE_SLOPE_CEILING_R: return y < x;
        case BLOCK_ATTR_OBS_TYPE_SLOPE_FLOOR_L: return y >= x;
        case BLOCK_ATTR_OBS_TYPE_SLOPE_FLOOR_R: return y >= 7 - x;
        default: return false;
    }
}

bool mapGetPixelObs(int16_t x, int16_t y) {
    return mapGetPixelAttributeObs(mapGetPixelAttribute(x, y), x, y);
}

void mapDraw() {
    int16_t drawX = -((mapCameraX % 16 + 16) % 16);
    int16_t startColumn = mapCameraX / 16;
    uint8_t columnCount = frogboy::SCREEN_WIDTH / 16 + 2;
    uint8_t mapTileWidth = frogboy::readRom<uint8_t>(mapWidths + mapCurrentIndex);

    if(mapTileWidth == 0) {
        return;
    }

    if(startColumn < 0) {
        startColumn = 0;
    }  
    if(startColumn > 255) {
        startColumn = 255;
    }

    uint8_t clampedStartColumn = static_cast<uint8_t>(startColumn);

    if(static_cast<uint8_t>(clampedStartColumn) > mapTileWidth - columnCount) {
        columnCount = mapTileWidth - static_cast<uint8_t>(clampedStartColumn);
    }

    const uint8_t* mapPtr = frogboy::readRom<const uint8_t*>(mapAddresses + mapCurrentIndex) + clampedStartColumn;
    for(uint8_t col = 0; col != columnCount; ++col) {
        int16_t drawY = mapCameraY;

        const uint8_t* columnPtr = columnBlocks + static_cast<uint16_t>(frogboy::readRom<uint8_t>(mapPtr++)) * 4;

        for(uint8_t row = 0; row != 4; ++row) {
            uint16_t blockIndex = static_cast<uint16_t>(frogboy::readRom<uint8_t>(columnPtr++)) * 4;
            const uint8_t* blockTilePtr = blockTiles + blockIndex;
            const uint8_t* blockAttrPtr = blockAttributes + blockIndex;

            for(uint8_t i = 0; i != 4; ++i) {
                uint8_t tile = frogboy::readRom<uint8_t>(blockTilePtr++);
                uint8_t attr = frogboy::readRom<uint8_t>(blockAttrPtr++);
                frogboy::drawTile(drawX + (i % 2 == 0 ? 0 : 8), drawY + (i / 2 == 0 ? 0 : 8), spritesBitmap, tile, 1, (attr & BLOCK_ATTR_HFLIP) != 0, (attr & BLOCK_ATTR_VFLIP) != 0);
            }

            drawY += 16;
        }

        drawX += 16;
    }
}

uint8_t mapGetWidth() {
    return frogboy::readRom<uint8_t>(mapWidths + mapCurrentIndex);
}