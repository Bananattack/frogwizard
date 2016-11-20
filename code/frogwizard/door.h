#ifndef DOOR_H
#define DOOR_H

enum DoorType {
    DOOR_TYPE_START,
    DOOR_TYPE_GRASSLAND_HOUSE,
    DOOR_TYPE_HOUSE_GRASSLAND,
    DOOR_TYPE_GRASSLAND_HOUSE2,
    DOOR_TYPE_HOUSE2_GRASSLAND,
    DOOR_TYPE_GRASSLAND_HOUSE3,
    DOOR_TYPE_HOUSE3_GRASSLAND,

    DOOR_TYPE_COUNT,
};

namespace door {
    void read(DoorType door, int16_t& x, int16_t& y, uint8_t& mapIndex);
}

#endif
