#include <stddef.h>
#include <string.h>
#include "frogboy.h"
#include "bullet.h"
#include "player.h"
#include "critter.h"
#include "sprite.h"
#include "game.h"
#include "particle.h"
#include "hitbox.h"

Bullet bullets[ENT_COUNT_BULLET];

typedef void (*BulletHandler)(Entity* ent, Bullet* bullet);
extern const uint8_t bulletDamage[BULLET_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const BulletHandler bulletInitHandler[BULLET_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const BulletHandler bulletUpdateHandler[BULLET_TYPE_COUNT] FROGBOY_ROM_DATA;

void bulletInitSystem() {
    memset(bullets, 0, sizeof(bullets));
}

Bullet* bulletAdd(int16_t x, int16_t y, bool dir, BulletType type) {
    Entity* ent = entityAdd(x, y, ENT_OFFSET_BULLET, ENT_COUNT_BULLET);

    if(ent != nullptr) {
        ptrdiff_t bulletIndex = ent - &ents[ENT_OFFSET_BULLET];
        Bullet* bullet = &bullets[bulletIndex];

        memset(bullet, 0, sizeof(Bullet));
        bullet->type = type;
        bullet->dir = dir;
        ent->hitbox = HITBOX_TYPE_BULLET_16x16;
        ent->controlFlags |= ENT_CTRL_FLAG_IGNORE_SLOPES;
        ent->drawFlags |= (dir ? ENT_DRAW_FLAG_HFLIP : 0);
        frogboy::readRom<BulletHandler>(&bulletInitHandler[bullet->type])(ent, bullet);
        return bullet;
    }

    return nullptr;
}

void bulletRemove(Entity* ent, Bullet* bullet) {
    if((bullet->flags & BULLET_FLAG_PLAYER_BULLET) != 0) {
        if(player.shotCount > 0) {
            player.shotCount--;
        }
    }

    bullet->type = 0;
    entityRemove(ent);
}

void bulletUpdateAll() {
    for(uint8_t bulletIndex = 0; bulletIndex != ENT_COUNT_BULLET; ++bulletIndex) {
        uint8_t entityIndex = ENT_OFFSET_BULLET + bulletIndex;
        Entity* ent = &ents[entityIndex];

        if((ent->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
            entityUpdate(ent);

            Bullet* bullet = &bullets[bulletIndex];

            uint8_t damage = frogboy::readRom<uint8_t>(&bulletDamage[bullet->type]);
            if(damage > 0) {
                for(uint8_t critterIndex = 0; critterIndex != ENT_COUNT_CRITTER; ++critterIndex) {
                    uint8_t targetEntIndex = ENT_OFFSET_CRITTER + critterIndex;
                    Entity* targetEnt = &ents[targetEntIndex];
                    Critter* critter = &critters[critterIndex];
                    if((targetEnt->controlFlags & ENT_CTRL_FLAG_BULLET_TARGET) != 0
                    && entityCollide(ent, 1, targetEnt, 1)) {
                        critterHurt(targetEnt, critter, damage);
                        bulletRemove(ent, bullet);
                        break;
                    }
                }
            }

            frogboy::readRom<BulletHandler>(&bulletUpdateHandler[bullet->type])(ent, bullet);

            if(!gameCheckOnScreen(ent->x / 16 + 8, ent->y / 16 + 8, 16, 16)) {
                bulletRemove(ent, bullet);
            }
        }
    }
}

void bulletDrawAll() {
    for(uint8_t bulletIndex = 0; bulletIndex != ENT_COUNT_BULLET; ++bulletIndex) {
        entityDraw(&ents[ENT_OFFSET_BULLET + bulletIndex]);
    }
}

void fireballInit(Entity* ent, Bullet* bullet) {
    bullet->flags |= BULLET_FLAG_PLAYER_BULLET;
    ent->xspd = bullet->dir ? 56 : -56;
    ent->sprite = SPRITE_TYPE_CIRCLE;    
}

void fireballUpdate(Entity* ent, Bullet* bullet) {
    if(bullet->timer >= 5) {
        ent->sprite = ((bullet->timer - 5) % 8) < 4 ? SPRITE_TYPE_FIREBALL_1 : SPRITE_TYPE_FIREBALL_2;
        ent->drawFlags &= ~ENT_DRAW_FLAG_FLASH;
    } else if(bullet->timer >= 2) {
        ent->drawFlags |= ENT_DRAW_FLAG_FLASH;
    }
    bullet->timer++;
    
    bool hitObs = (ent->status & ENT_STATUS_HIT_OBS) != 0;

    if(hitObs) {
        int16_t x = ent->x + (bullet->dir ? 64 : -64);
        int16_t y = ent->y;
        bulletRemove(ent, bullet);

        if(hitObs) {
            bulletAdd(x, y, false, BULLET_TYPE_EXPLOSION);
        }
        return;
    }
}

void explosionInit(Entity* ent, Bullet* bullet) {
    static_cast<void>(bullet);
    ent->controlFlags |= ENT_CTRL_FLAG_IGNORE_OBS;
    ent->sprite = SPRITE_TYPE_CIRCLE;
}

void explosionUpdate(Entity* ent, Bullet* bullet) {
    if(bullet->timer >= 8) {
        bulletRemove(ent, bullet);
    } else if(bullet->timer >= 2) {
        ent->drawFlags |= ENT_DRAW_FLAG_FLASH;
    }
    bullet->timer++;
}

const uint8_t bulletDamage[BULLET_TYPE_COUNT] FROGBOY_ROM_DATA = {
    1,
    0,
};

const BulletHandler bulletInitHandler[BULLET_TYPE_COUNT] FROGBOY_ROM_DATA = {
    fireballInit,
    explosionInit,
};

const BulletHandler bulletUpdateHandler[BULLET_TYPE_COUNT] FROGBOY_ROM_DATA = {
    fireballUpdate,
    explosionUpdate,
};
