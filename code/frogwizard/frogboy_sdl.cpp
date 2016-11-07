#ifdef FROGBOY_SDL
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL.h>

#include "frogboy.h"

namespace {
    bool open = false;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* screenTexture = nullptr;
    SDL_Surface* screenSurface = nullptr;
    uint8_t screenBuffer[frogboy::SCREEN_WIDTH * (frogboy::SCREEN_HEIGHT / 8)];
    uint32_t deltaTime = 0;
    uint32_t lastFrame = 0;

    const uint8_t SCREEN_SCALE = 8;

    bool windowMaximized = false;

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
        window = SDL_CreateWindow(FROGBOY_APPNAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * SCREEN_SCALE, SCREEN_HEIGHT * SCREEN_SCALE, SDL_WINDOW_RESIZABLE);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
        screenSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
        clearScreen();
        lastFrame = SDL_GetTicks();

        memset(pressed, 0, sizeof(pressed));

        windowMaximized = false;
        open = true;
    }

    void destroy() {
        SDL_FreeSurface(screenSurface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void clearScreen() {
        memset(screenBuffer, 0, sizeof(screenBuffer));
    }

    void refreshScreen() {
        SDL_LockSurface(screenSurface);
        uint32_t black = SDL_MapRGBA(screenSurface->format, 0, 0, 0, 0);
        uint32_t white = SDL_MapRGBA(screenSurface->format, 255, 255, 255, 0);
        uint32_t* surfaceRowPtr = static_cast<uint32_t*>(screenSurface->pixels);
        for(size_t y = 0; y != SCREEN_HEIGHT; ++y) {
            uint32_t* ptr = surfaceRowPtr;
            for(size_t x = 0; x != SCREEN_WIDTH; ++x) {
                size_t shift = y % 8;
                size_t bufferRow = y / 8;
                *ptr = ((screenBuffer[bufferRow * SCREEN_WIDTH + x] >> shift) & 1) == 0 ? black : white;
                ptr++;
            }
            surfaceRowPtr = static_cast<uint32_t*>(static_cast<void*>(
                    static_cast<uint8_t*>(static_cast<void*>(surfaceRowPtr)) + screenSurface->pitch));
        }
        SDL_UnlockSurface(screenSurface);

        SDL_UpdateTexture(screenTexture, NULL, screenSurface->pixels, screenSurface->pitch);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    bool waitForFrame() {
        if(!open) {
            return false;
        }

        bool resized = false;

        {
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
                    case SDL_WINDOWEVENT:
                        switch(e.window.event) {
                            case SDL_WINDOWEVENT_RESIZED: {
                                resized = true;
                                break;
                            }
                            case SDL_WINDOWEVENT_MAXIMIZED: {
                                windowMaximized = true;
                                break;
                            }
                            case SDL_WINDOWEVENT_RESTORED: {
                                windowMaximized = false;
                                resized = true;
                                break;
                            }
                        }
                }
            }
        }

        if(!windowMaximized && resized) {
            int width, height;
            SDL_GetWindowSize(window, &width, &height);
            int xscale = (width + SCREEN_WIDTH / 2) / SCREEN_WIDTH;
            int yscale = (height + SCREEN_WIDTH / 2) / SCREEN_WIDTH;

            if(width % SCREEN_WIDTH != 0
            || height % SCREEN_HEIGHT != 0
            || xscale != yscale) {
                int scale = xscale;
                if(scale < yscale) {
                    scale = yscale;
                }
                if(scale < 1) {
                    scale = 1;
                }
                SDL_SetWindowSize(window, scale * SCREEN_WIDTH, scale * SCREEN_HEIGHT);
            }
        }

        if(open) {
            uint32_t timer = SDL_GetTicks();
            deltaTime += timer - lastFrame;
            lastFrame = timer;

            if(deltaTime > 48) {
                deltaTime = 48;
            }
            if(deltaTime >= 16) {
                deltaTime -= 16;
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