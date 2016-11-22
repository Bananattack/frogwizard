#include <stddef.h>
#include <string.h>
#include "frogboy.h"
#include "bullet.h"
#include "player.h"
#include "critter.h"
#include "sprite.h"
#include "particle.h"
#include "hitbox.h"
#include "camera.h"

typedef void (*BulletHandler)(Entity* ent, Bullet* bullet);
extern const uint8_t bulletDamage[BULLET_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const BulletHandler bulletInitHandler[BULLET_TYPE_COUNT] FROGBOY_ROM_DATA;
extern const BulletHandler bulletUpdateHandler[BULLET_TYPE_COUNT] FROGBOY_ROM_DATA;

Bullet Bullet::data[ENT_COUNT_BULLET];

void Bullet::initSystem() {
    memset(data, 0, sizeof(data));
}

Bullet* Bullet::add(int16_t x, int16_t y, bool dir, BulletType type) {
    Entity* ent = Entity::add(x, y, ENT_OFFSET_BULLET, ENT_COUNT_BULLET);

    if(ent != nullptr) {
        ptrdiff_t bulletIndex = ent - &Entity::data[ENT_OFFSET_BULLET];
        Bullet* bullet = &data[bulletIndex];

        memset(bullet, 0, sizeof(Bullet));
        bullet->type = type;
        bullet->dir = dir;
        ent->hitbox = HITBOX_TYPE_FIREBALL;
        ent->controlFlags |= ENT_CTRL_FLAG_IGNORE_SLOPES;
        ent->drawFlags |= (dir ? ENT_DRAW_FLAG_HFLIP : 0);
        frogboy::readRom<BulletHandler>(&bulletInitHandler[bullet->type])(ent, bullet);
        return bullet;
    }

    return nullptr;
}

void Bullet::updateAll() {
    Bullet* bullet = &data[0];
    Bullet* bulletEnd = bullet + ENT_COUNT_BULLET;
    Entity* ent = &Entity::data[ENT_OFFSET_BULLET];
    for(; bullet != bulletEnd; ++bullet, ++ent) {
        if((ent->controlFlags & ENT_CTRL_FLAG_ACTIVE) != 0) {
            ent->update();

            uint8_t damage = frogboy::readRom<uint8_t>(&bulletDamage[bullet->type]);
            if(damage > 0) {
                Critter* critter = &Critter::data[0];
                Critter* critterEnd = critter + ENT_COUNT_CRITTER;
                Entity* targetEnt = &Entity::data[ENT_OFFSET_CRITTER];
                for(; critter != critterEnd; ++critter, ++targetEnt) {
                    if((targetEnt->controlFlags & ENT_CTRL_FLAG_BULLET_TARGET) != 0
                    && ent->collide(1, targetEnt, 1)) {
                        critter->hurt(targetEnt, damage);
                        bullet->remove(ent);
                        break;
                    }
                }
            }

            frogboy::readRom<BulletHandler>(&bulletUpdateHandler[bullet->type])(ent, bullet);

            if(!camera.containsPoint(ent->x / 16 + 8, ent->y / 16 + 8, 16, 16)) {
                bullet->remove(ent);
            }
        }
    }
}

void Bullet::drawAll() {
    for(uint8_t bulletIndex = 0; bulletIndex != ENT_COUNT_BULLET; ++bulletIndex) {
        Entity::data[ENT_OFFSET_BULLET + bulletIndex].draw();
    }
}

void Bullet::remove(Entity* ent) {
    if((flags & BULLET_FLAG_PLAYER_BULLET) != 0) {
        if(player.instance.shotCount > 0) {
            player.instance.shotCount--;
        }
    }

    type = 0;
    ent->remove();
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
        bullet->remove(ent);

        if(hitObs) {
            frogboy::playTone(150, 10);
            Bullet::add(x, y, false, BULLET_TYPE_EXPLOSION);
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
        bullet->remove(ent);
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
