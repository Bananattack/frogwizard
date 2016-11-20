#include <string.h>
#include "spawn.h"
#include "frogboy.h"
#include "critter.h"
#include "map.h"
#include "door.h"
#include "text.h"
#include "camera.h"

namespace {
    uint8_t spawnUsed[SPAWN_BIT_ARRAY_SIZE];
    uint8_t spawnKilled[SPAWN_BIT_ARRAY_SIZE];
    
    struct SuspendedSpawn {
        int16_t x;
        int16_t y;
    };

    SuspendedSpawn suspendedSpawns[SPAWN_MAX];

    const uint8_t FROGBOY_ROM_DATA grasslandSpawnData[] = {
        10, 1, CRITTER_TYPE_DOOR, DOOR_TYPE_GRASSLAND_HOUSE,
        16, 2, CRITTER_TYPE_BLOCK, 0,
        24, 1, CRITTER_TYPE_WALKER, 0,
        49, 1, CRITTER_TYPE_BLOCK, 0,
        40, 1, CRITTER_TYPE_DOOR, DOOR_TYPE_GRASSLAND_HOUSE2,
        67, 2, CRITTER_TYPE_DOOR, DOOR_TYPE_GRASSLAND_HOUSE3,
    };

    const uint8_t houseSpawnData[] FROGBOY_ROM_DATA = {
        2, 2, CRITTER_TYPE_DOOR, DOOR_TYPE_HOUSE_GRASSLAND,
        2, 1, CRITTER_TYPE_LABEL, TEXT_TYPE_FIND_EGG,
        4, 2, CRITTER_TYPE_EGG, TEXT_TYPE_FIND_EGG,
    };

    const uint8_t house2SpawnData[] FROGBOY_ROM_DATA = {
        5, 2, CRITTER_TYPE_DOOR, DOOR_TYPE_HOUSE2_GRASSLAND,
    };

    const uint8_t house3SpawnData[] FROGBOY_ROM_DATA = {
        2, 2, CRITTER_TYPE_DOOR, DOOR_TYPE_HOUSE3_GRASSLAND,
    };

    const uint8_t spawnCounts[MAP_TYPE_COUNT] FROGBOY_ROM_DATA = {
        sizeof(grasslandSpawnData) / 4,
        sizeof(houseSpawnData) / 4,
        sizeof(house2SpawnData) / 4,
        sizeof(house3SpawnData) / 4,
    };

    const uint8_t* const spawnPointers[MAP_TYPE_COUNT] FROGBOY_ROM_DATA = {
        grasslandSpawnData,
        houseSpawnData,
        house2SpawnData,
        house3SpawnData,
    };
}

namespace spawn {
    void init() {
        memset(spawnUsed, 0, sizeof(spawnUsed));
        memset(spawnKilled, 0, sizeof(spawnKilled));
        for(uint8_t i = 0; i != SPAWN_MAX; ++i) {
            suspendedSpawns[i].x = suspendedSpawns[i].y = 0x7FFF;
        }
    }

    void suspend(uint8_t index, int16_t x, int16_t y) {
        if(index < SPAWN_MAX) {
            uint8_t mask = 1 << (index % 8);
            spawnUsed[index / 8] &= ~mask;
            spawnKilled[index / 8] |= mask;
            suspendedSpawns[index].x = x;
            suspendedSpawns[index].y = y;
        }
    }

    void check() {
        uint8_t count = frogboy::readRom<uint8_t>(&spawnCounts[map.currentIndex]);
        const uint8_t* spawnPtr = frogboy::readRom<const uint8_t*>(&spawnPointers[map.currentIndex]);

        for(uint8_t i = 0; i != count; ++i) {
            int16_t x = static_cast<int16_t>(frogboy::readRom<uint8_t>(spawnPtr++)) * 16;
            int16_t y = static_cast<int16_t>(frogboy::readRom<uint8_t>(spawnPtr++)) * 16;
            uint8_t type = frogboy::readRom<uint8_t>(spawnPtr++);
            uint8_t data = frogboy::readRom<uint8_t>(spawnPtr++);
            uint8_t mask = 1 << (i % 8);

            if(Critter::canPersistType(static_cast<CritterType>(type))
            && suspendedSpawns[i].x != 0x7FFF) {
                x = suspendedSpawns[i].x;
                y = suspendedSpawns[i].y;
            }

            if(camera.containsPoint(x + 8, y + 8, 64, 16)) {
                if((spawnUsed[i / 8] & mask) == 0
                && (spawnKilled[i / 8] & mask) == 0) {
                    Critter* critter = Critter::add(x * 16, y * 16, static_cast<CritterType>(type), data);
                    if(critter != nullptr) {
                        critter->spawnIndex = i;
                        spawnUsed[i / 8] |= mask;
                    }
                }
            } else {
                spawnKilled[i / 8] &= ~mask;
            }
        }
    }
}