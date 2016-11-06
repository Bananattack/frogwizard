#ifndef SPRITES_BITMAP_H
#define SPRITES_BITMAP_H

#include <avr/pgmspace.h>
#include <stdint.h>

#define SPRITES_BITMAP_LEN 4096
extern const uint8_t spritesBitmap[SPRITES_BITMAP_LEN] PROGMEM;
#endif
