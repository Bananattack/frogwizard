#include "frogboy.h"
#include "hitbox.h"

// x y w h
static const int8_t hitboxData[(int) HITBOX_TYPE_COUNT * 4] FROGBOY_ROM_DATA = {
    0, 0, 0, 0,
    0, 0, 16, 16,
    0, 0, 8, 8,
    4, 4, 8, 8,
    1, 4, 14, 12,
};

bool hitboxCollide(int16_t xa, int16_t ya, HitboxType hitboxA, int8_t borderA, int16_t xb, int16_t yb, HitboxType hitboxB, int8_t borderB) {
    int8_t hxa, hya, hwa, hha;
    hitboxGetData(hitboxA, &hxa, &hya, &hwa, &hha);
    xa += hxa;
    ya += hya;

    int8_t hxb, hyb, hwb, hhb;
    hitboxGetData(hitboxB, &hxb, &hyb, &hwb, &hhb);
    xb += hxb;
    yb += hyb;

    return xa - borderA < xb + hwb + borderB
        && xa + hwa + borderA > xb - borderB
        && ya - borderA < yb + hhb + borderB
        && ya + hha + borderA > yb - borderB;
}

void hitboxGetData(HitboxType hitbox, int8_t* x, int8_t* y, int8_t* w, int8_t* h) {
    const int8_t* hitboxPtr = hitboxData + static_cast<uint16_t>(hitbox) * 4;
    *x = frogboy::readRom<int8_t>(hitboxPtr++);
    *y = frogboy::readRom<int8_t>(hitboxPtr++);
    *w = frogboy::readRom<int8_t>(hitboxPtr++);
    *h = frogboy::readRom<int8_t>(hitboxPtr++);
}