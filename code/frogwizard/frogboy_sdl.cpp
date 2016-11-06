#ifdef FROGBOY_SDL
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL.h>

#include "frogboy.h"

namespace {
    bool open = false;
    SDL_Window* window = nullptr;
    SDL_Surface* windowSurface = nullptr;
    SDL_Surface* backSurface = nullptr;
    uint8_t screenBuffer[frogboy::SCREEN_WIDTH * (frogboy::SCREEN_HEIGHT / 8)];
    uint32_t lastFrame = 0;

    const uint8_t SCREEN_SCALE = 8;

    bool pressed[static_cast<size_t>(frogboy::BUTTON_COUNT)];

    const SDL_Keycode keycodes[static_cast<size_t>(frogboy::BUTTON_COUNT)] = {
        SDLK_LEFT,
        SDLK_RIGHT,
        SDLK_UP,
        SDLK_DOWN, 
        SDLK_z,
        SDLK_x,
    };
}

namespace frogboy {
    void init() {
        srand(static_cast<unsigned int>(time(nullptr)));

        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
        window = SDL_CreateWindow(FROGBOY_APPNAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * SCREEN_SCALE, SCREEN_HEIGHT * SCREEN_SCALE, 0);
        windowSurface = SDL_GetWindowSurface(window);
        backSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
        clearScreen();
        lastFrame = SDL_GetTicks();

        memset(pressed, 0, sizeof(pressed));

        open = true;
    }

    void destroy() {
        SDL_FreeSurface(backSurface);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void clearScreen() {
        memset(screenBuffer, 0, sizeof(screenBuffer));
    }

    void refreshScreen() {
        SDL_LockSurface(backSurface);
        uint32_t black = SDL_MapRGBA(backSurface->format, 0, 0, 0, 0);
        uint32_t white = SDL_MapRGBA(backSurface->format, 255, 255, 255, 0);
        uint32_t* surfaceRowPtr = static_cast<uint32_t*>(backSurface->pixels);
        for(size_t y = 0; y != SCREEN_HEIGHT; ++y) {
            uint32_t* ptr = surfaceRowPtr;
            for(size_t x = 0; x != SCREEN_WIDTH; ++x) {
                size_t shift = y % 8;
                size_t bufferRow = y / 8;
                *ptr = ((screenBuffer[bufferRow * SCREEN_WIDTH + x] >> shift) & 1) == 0 ? black : white;
                ptr++;
            }
            surfaceRowPtr = static_cast<uint32_t*>(static_cast<void*>(
                    static_cast<uint8_t*>(static_cast<void*>(surfaceRowPtr)) + backSurface->pitch));
        }
        SDL_UnlockSurface(backSurface);

        SDL_FillRect(windowSurface, NULL, black);

        SDL_Rect destRect;
        destRect.x = 0;
        destRect.y = 0;
        destRect.w = SCREEN_WIDTH * SCREEN_SCALE;
        destRect.h = SCREEN_HEIGHT * SCREEN_SCALE;        
        SDL_BlitScaled(backSurface, nullptr, windowSurface, &destRect);

        SDL_UpdateWindowSurface(window);
    }

    bool waitForFrame() {
        if(!open) {
            return false;
        }

        SDL_Event e;
        while(SDL_PollEvent(&e) != 0) {
            switch(e.type) {
                case SDL_QUIT:
                    open = false;
                    break;
                case SDL_KEYDOWN:
                    for(size_t i = 0; i != BUTTON_COUNT; ++i) {
                        if(e.key.keysym.sym == keycodes[i]) {
                            pressed[i] = true;
                            break;
                        }
                    }
                    break;
                case SDL_KEYUP:
                    for(size_t i = 0; i != BUTTON_COUNT; ++i) {
                        if(e.key.keysym.sym == keycodes[i]) {
                            pressed[i] = false;
                            break;
                        }
                    }
                    break;
            }
        }

        if(open) {
            uint32_t timer = SDL_GetTicks();
            if(timer - lastFrame > 16) {
                lastFrame = timer;
                return true;
            }
        }

        return false;
    }

    bool isActive() {
        return open;
    }    

    uint8_t reverseBits(uint8_t value) {
        value = (value & 0xF0) >> 4 | (value & 0x0F) << 4;
        value = (value & 0xCC) >> 2 | (value & 0x33) << 2;
        value = (value & 0xAA) >> 1 | (value & 0x55) << 1;
        return value;
    }

    uint8_t* getScreenBuffer() {
        return screenBuffer;
    }

    bool isPressed(Button button) {
        return pressed[static_cast<size_t>(button)];
    }

    int getRandom(int min, int max) {
        return rand() % (max - min + 1) + min;
    }
}
#endif