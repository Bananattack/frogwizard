#ifndef SPRITES_BITMAP_H
#define SPRITES_BITMAP_H

#include <stdint.h>

#include "frogboy.h"

#define SPRITES_BITMAP_LEN 4096
extern const uint8_t spritesBitmap[SPRITES_BITMAP_LEN] FROGBOY_ROM_DATA;
#endif
