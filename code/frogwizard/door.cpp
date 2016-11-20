#include <stdint.h>
#include "frogboy.h"
#include "map.h"
#include "door.h"

namespace {
    const uint8_t doorData[DOOR_TYPE_COUNT * 3] FROGBOY_ROM_DATA = {
        2, 2, MAP_TYPE_GRASSLAND,
        2, 2, MAP_TYPE_HOUSE,
        10, 1, MAP_TYPE_GRASSLAND,
        5, 2, MAP_TYPE_HOUSE2,
        40, 1, MAP_TYPE_GRASSLAND,
        2, 2, MAP_TYPE_HOUSE3,
        67, 2, MAP_TYPE_GRASSLAND,
    };
}

namespace door {
    void read(DoorType door, int16_t& x, int16_t& y, uint8_t& currentIndex) {
        const uint8_t* doorPtr = &doorData[door * 3];
        x = frogboy::readRom<uint8_t>(doorPtr++) * 256;
        y = frogboy::readRom<uint8_t>(doorPtr++) * 256;
        currentIndex = frogboy::readRom<uint8_t>(doorPtr++);
    }
}
