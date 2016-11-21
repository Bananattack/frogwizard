#include "save.h"
#include "player.h"
#include "frogboy.h"

namespace {
    const uint8_t SIGNATURE[3] = {'E', 'G', 'G'};

    bool readAndValidateHeader(frogboy::Save* save) {
        uint8_t signature[3];
        frogboy::saveReadBytes(save, 3, signature);
        if(signature[0] == SIGNATURE[0]
        && signature[1] == SIGNATURE[1]
        && signature[2] == SIGNATURE[2]) {
            return true;
        }

        return false;
    }
}

namespace save {
    bool exists() {
        bool valid = false;
        frogboy::Save* save = frogboy::saveOpen(frogboy::SAVE_OPEN_MODE_READ);
        if(save != nullptr) {
            valid = readAndValidateHeader(save);            
            frogboy::saveClose(save);
        }
        return valid;
    }

    bool load() {
        bool valid = false;
        frogboy::Save* save = frogboy::saveOpen(frogboy::SAVE_OPEN_MODE_READ);
        if(save != nullptr) {
            valid = readAndValidateHeader(save);
            frogboy::saveReadBytes(save, 1, &player.status.lastDoor);

            uint8_t flags;
            frogboy::saveReadBytes(save, 1, &flags);
            player.status.dir = (flags & 1) != 0;

            frogboy::saveReadBytes(save, EGG_BIT_ARRAY_SIZE, player.status.eggs);
            frogboy::saveClose(save);
        }
        return valid;
    }

    void save() {
        frogboy::Save* save = frogboy::saveOpen(frogboy::SAVE_OPEN_MODE_WRITE);
        if(save != nullptr) {
            frogboy::saveWriteBytes(save, 3, SIGNATURE);
            frogboy::saveWriteBytes(save, 1, &player.status.lastDoor);

            uint8_t flags = (player.status.dir ? 1 : 0) << 0;
            frogboy::saveWriteBytes(save, 1, &flags);

            frogboy::saveWriteBytes(save, EGG_BIT_ARRAY_SIZE, player.status.eggs);
            frogboy::saveClose(save);
        }
    }
}