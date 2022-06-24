#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "dbg.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320
#define FPS 60

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
uint32_t *color_buffer;

bool setup(void) {
    check(SDL_Init(SDL_INIT_VIDEO) == 0, "Faild to initialize SDL.\n");

    window = SDL_CreateWindow(
        NULL,                   // window title
        SDL_WINDOWPOS_CENTERED, // x pos
        SDL_WINDOWPOS_CENTERED, // y pos
        WINDOW_WIDTH,           // width
        WINDOW_HEIGHT,          // height
        SDL_WINDOW_BORDERLESS   // type
    );
    check(window, "Failed to create SDL window.\n");

    // SDL Renderer
    renderer = SDL_CreateRenderer(window, -1, 0);
    check(renderer, "Failed to create SDL renderer.\n");

    color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * WINDOW_WIDTH * WINDOW_HEIGHT);
    check_mem(color_buffer);

    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            WINDOW_WIDTH,
            WINDOW_HEIGHT
    );
    check(texture, "Failed to create SDL texture.\n");

    return true;

error:
    return false;
}

void update(void) {
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        color_buffer[i] = 0xFFFFFFFF;
    }
}

void render(void) {
    SDL_UpdateTexture(
            texture,
            NULL,
            color_buffer,
            (int)(WINDOW_WIDTH * sizeof(uint32_t))
    );
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    SDL_RenderPresent(renderer);
}

void game_loop(void) {
    /* process_input(); */
    update();
    render();
}

int main(int argc, char *argv[]) {
    bool initialized = setup();
    if(!initialized) {
        printf("Init failed.\n");
        exit(1);
    }

    #ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(render, 60, 1);
    #else
    while(1) {
        render();
        SDL_Delay(16);
    }
    #endif
}
