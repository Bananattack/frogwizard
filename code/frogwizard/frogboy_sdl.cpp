#ifdef FROGBOY_SDL
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL.h>

#include "frogboy.h"

namespace {
    enum {
        FRAME_INTERVAL = 16,
        SCREEN_SCALE = 8,
        DESKTOP_FIT_BORDER = 64,

        AUDIO_CHANNEL_COUNT = 2,
        AUDIO_FREQUENCY = 48000,
        AUDIO_AMPLITUDE = 15000,
    };

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* screenTexture = nullptr;
    SDL_Surface* screenSurface = nullptr;
    uint8_t screenBuffer[frogboy::SCREEN_WIDTH * (frogboy::SCREEN_HEIGHT / 8)];

    uint32_t deltaTime = 0;
    uint32_t lastFrame = 0;
    uint32_t lastMouseMove = 0;

    bool windowOpen = false;
    bool windowMaximized = false;
    bool windowFullscreen = false;

    bool pressed[static_cast<size_t>(frogboy::BUTTON_COUNT)];

    struct KeyCode {
        SDL_Keycode code;
        uint16_t mod;
    };

    const KeyCode keycodes[static_cast<size_t>(frogboy::BUTTON_COUNT)] = {
        {SDLK_LEFT, 0},
        {SDLK_RIGHT, 0},
        {SDLK_UP, 0},
        {SDLK_DOWN, 0},
        {SDLK_z, 0},
        {SDLK_x, 0},
        {SDLK_RETURN, 0},
        {SDLK_r, KMOD_CTRL},
    };

    SDL_AudioSpec audioSpecRequested;
    SDL_AudioSpec audioSpecActual;
    SDL_AudioDeviceID audioDevice;

    uint8_t audioChannelNextIndex;

    struct AudioChannel {
        uint32_t sampleCount;
        uint16_t frequency;
        double phase;
    };

    AudioChannel audioChannels[AUDIO_CHANNEL_COUNT];

    void audioCallback(void* userdata, uint8_t* rawBuffer, int rawBufferLength) {
        int16_t* samples = reinterpret_cast<int16_t*>(rawBuffer);
        int sampleCount = rawBufferLength / 2;

        for(int sampleIndex = 0; sampleIndex != sampleCount; ++sampleIndex) {
            int16_t sample = 0;
            for(int channelIndex = 0; channelIndex != AUDIO_CHANNEL_COUNT; ++channelIndex) {
                if(audioChannels[channelIndex].sampleCount > 0) {
                    audioChannels[channelIndex].sampleCount--;
                    sample += sin(static_cast<double>(audioChannels[channelIndex].phase) / static_cast<double>(AUDIO_FREQUENCY) * 2 * M_PI) < 0 ? -AUDIO_AMPLITUDE : AUDIO_AMPLITUDE;
                    audioChannels[channelIndex].phase += audioChannels[channelIndex].frequency;
                }
            }

            samples[sampleIndex] = sample;
        }
    }
}

namespace frogboy {
    bool init() {
        srand(static_cast<unsigned int>(time(nullptr)));

        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
            return false;
        }

        int scale = SCREEN_SCALE;
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {            
            if(dm.w - DESKTOP_FIT_BORDER < SCREEN_WIDTH * scale
            || dm.h - DESKTOP_FIT_BORDER < SCREEN_HEIGHT * scale) {
                int xscale = (dm.w - DESKTOP_FIT_BORDER) / SCREEN_WIDTH;
                int yscale = (dm.h - DESKTOP_FIT_BORDER) / SCREEN_HEIGHT;
                scale = xscale < yscale ? xscale : yscale;

                if(scale < 1) {
                    scale = 1;
                }
            }
        }

        window = SDL_CreateWindow(FROGBOY_APPNAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * scale, SCREEN_HEIGHT * scale, SDL_WINDOW_RESIZABLE);
        if(window == nullptr) {
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
        if(renderer == nullptr) {
            return false;
        }

        SDL_SetWindowGrab(window, SDL_FALSE);
        SDL_SetRelativeMouseMode(SDL_FALSE);

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

        screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
        if(screenTexture == nullptr) {
            return false;
        }

        screenSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
        if(screenSurface == nullptr) {
            return false;
        }

        memset(&audioSpecRequested, 0, sizeof(audioSpecRequested));
        audioSpecRequested.freq = AUDIO_FREQUENCY;
        audioSpecRequested.format = AUDIO_S16;
        audioSpecRequested.channels = 1;
        audioSpecRequested.samples = 2048;
        audioSpecRequested.callback = audioCallback;

        memset(audioChannels, 0, sizeof(audioChannels));
        audioChannelNextIndex = 0;

        audioDevice = SDL_OpenAudioDevice(nullptr, 0, &audioSpecRequested, &audioSpecActual, 0);
        if(audioDevice == 0) {
            return false;
        }

        clearScreen();
        lastFrame = SDL_GetTicks();
        lastMouseMove = lastFrame;

        windowOpen = true;
        windowMaximized = false;
        windowFullscreen = false;

        memset(pressed, 0, sizeof(pressed));

        SDL_PauseAudioDevice(audioDevice, 0);

        return true;
    }

    void destroy() {
        if(audioDevice > 0) {
            SDL_CloseAudioDevice(audioDevice);
        }
        if(screenTexture != nullptr) {
            SDL_DestroyTexture(screenTexture);
        }
        if(screenSurface != nullptr) {
            SDL_FreeSurface(screenSurface);
        }
        if(renderer != nullptr) {
            SDL_DestroyRenderer(renderer);
        }
        if(window != nullptr) {
            SDL_DestroyWindow(window);
        }
        screenTexture = nullptr;
        screenSurface = nullptr;
        renderer = nullptr;
        window = nullptr;
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
            surfaceRowPtr = reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(surfaceRowPtr) + screenSurface->pitch);
        }
        SDL_UnlockSurface(screenSurface);

        SDL_UpdateTexture(screenTexture, NULL, screenSurface->pixels, screenSurface->pitch);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    void toggleFullscreen() {
        windowFullscreen = !windowFullscreen;
        SDL_SetWindowFullscreen(window, windowFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
        SDL_ShowCursor(windowFullscreen ? SDL_DISABLE : SDL_ENABLE);
        lastMouseMove = 0;
    }

    bool waitForFrame() {
        if(!windowOpen) {
            return false;
        }

        bool resized = false;

        {
            SDL_Event e;
            while(SDL_PollEvent(&e) != 0) {
                switch(e.type) {
                    case SDL_QUIT:
                        windowOpen = false;
                        break;
                    case SDL_KEYDOWN:
                        for(size_t i = 0; i != BUTTON_COUNT; ++i) {
                            const auto& keycode = keycodes[i];
                            if((keycode.mod == 0
                                || (e.key.keysym.mod & keycode.mod) != 0)
                            && e.key.keysym.sym == keycode.code) {
                                pressed[i] = true;
                                break;
                            }
                        }

                        if(e.key.repeat == 0
                        && ((e.key.keysym.mod & KMOD_ALT) != 0 && e.key.keysym.sym == SDLK_RETURN
                            || e.key.keysym.sym == SDLK_F11)) {
                            toggleFullscreen();
                        }

                        if(e.key.repeat == 0
                        && e.key.keysym.sym == SDLK_ESCAPE) {
                            if(windowFullscreen) {
                                toggleFullscreen();
                            }
                        }
                        break;
                    case SDL_KEYUP:
                        for(size_t i = 0; i != BUTTON_COUNT; ++i) {
                            const auto& keycode = keycodes[i];
                            if((keycode.mod == 0 || (e.key.keysym.mod & keycode.mod) != 0)
                            && e.key.keysym.sym == keycode.code) {
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
                        break;
                    case SDL_MOUSEMOTION:
                        lastMouseMove = SDL_GetTicks();
                        SDL_ShowCursor(SDL_ENABLE);
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                        if(e.button.button == SDL_BUTTON_LEFT
                        && e.button.clicks == 2) {
                            toggleFullscreen();                            
                        }
                }
            }
        }

        if(!windowFullscreen && !windowMaximized && resized) {
            int width, height;
            SDL_GetWindowSize(window, &width, &height);
            int xscale = (width + SCREEN_WIDTH / 2) / SCREEN_WIDTH;
            int yscale = (height + SCREEN_HEIGHT / 2) / SCREEN_HEIGHT;

            if(width % SCREEN_WIDTH != 0
            || height % SCREEN_HEIGHT != 0
            || xscale != yscale) {
                int scale = xscale > yscale ? xscale : yscale;
                if(scale < 1) {
                    scale = 1;
                }

                SDL_SetWindowSize(window, scale * SCREEN_WIDTH, scale * SCREEN_HEIGHT);
            }
        }

        if(windowOpen) {
            uint32_t timer = SDL_GetTicks();
            if(lastFrame + FRAME_INTERVAL > timer
            && deltaTime < FRAME_INTERVAL) {
                SDL_Delay(10);
            }

            timer = SDL_GetTicks();
            deltaTime += timer - lastFrame;
            lastFrame = timer;

            if(windowFullscreen && timer - lastMouseMove > 3000) {
                SDL_ShowCursor(SDL_DISABLE);
            }

            if(deltaTime > FRAME_INTERVAL * 3) {
                deltaTime = FRAME_INTERVAL * 3;
            }

            if(deltaTime >= FRAME_INTERVAL) {
                deltaTime -= FRAME_INTERVAL;
                return true;
            }
        }

        return false;
    }

    bool isActive() {
        return windowOpen;
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

    void playTone(uint16_t frequency, uint32_t duration) {
        SDL_LockAudioDevice(audioDevice);
        audioChannels[audioChannelNextIndex].frequency = frequency;
        audioChannels[audioChannelNextIndex].sampleCount = duration * AUDIO_FREQUENCY / 1000;
        audioChannels[audioChannelNextIndex].phase = 0;
        SDL_UnlockAudioDevice(audioDevice);

        audioChannelNextIndex = (audioChannelNextIndex + 1) % AUDIO_CHANNEL_COUNT;
    }

    bool isPressed(Button button) {
        return pressed[button];
    }

    int getRandom(int min, int max) {
        return rand() % (max - min + 1) + min;
    }
}
#endif