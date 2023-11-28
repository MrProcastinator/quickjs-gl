#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <EGL/egl.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool native_loaded;
#if defined(SDL_VIDEO_DRIVER_X11)
    Window native_window;
#else
    void* native_window;
#endif
    SDL_Window* window;
    SDL_Surface* surface;
} NativeDisplay;

static NativeDisplay currentDisplay;

static SDL_Window* createSDLWindow(int width, int height) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Failed to initialize the SDL2 library\n");
        exit(1);
    }

    SDL_Window *window = SDL_CreateWindow("SDL2 Window",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          width, height,
                                          0);
    if(!window)
    {
        fprintf(stderr, "Failed to create window\n");
        exit(1);
    }

    return window;
}

static void destroySDLWindow(SDL_Window* window)
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void createNativeDisplay(int width, int height) {
    SDL_SysWMinfo wmInfo;
    SDL_Window* window = createSDLWindow(width, height);

    SDL_VERSION(&wmInfo.version);
    if(!SDL_GetWindowWMInfo(window, &wmInfo)) {
        currentDisplay.native_loaded = false;
    } else {
        currentDisplay.native_loaded = true;
#if defined(SDL_VIDEO_DRIVER_X11)
        currentDisplay.native_window = wmInfo.info.x11.window;
#elif defined(__vita__)
        currentDisplay.native_window = NULL;
#endif
    }

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);

    if(!window_surface)
    {
        fprintf(stderr, "Failed to get the surface from the window\n");
        exit(1);
    }

    currentDisplay.window = window;
    currentDisplay.surface = window_surface;
}

void destroyNativeDisplay() {
    destroySDLWindow(currentDisplay.window);
}

void* getNativeWindow() {
    return 
        currentDisplay.native_loaded 
        ? 
#if defined(SDL_VIDEO_DRIVER_X11)
            &currentDisplay.native_window 
#else
            NULL
#endif
        : NULL
    ;
}

void handleNativeEventLoop() {
    SDL_Event event;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
        }
        SDL_Delay(100);
    }
}