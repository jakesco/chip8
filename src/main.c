#include <stdio.h>
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "dbg.h"
#include "cpu.h"

#define BUFFER_WIDTH 64
#define BUFFER_HEIGHT 32
#define SCALE_FACTOR 10

#define WINDOW_WIDTH (BUFFER_WIDTH * SCALE_FACTOR)
#define WINDOW_HEIGHT (BUFFER_HEIGHT * SCALE_FACTOR)

#define FPS 60
#define TARGET_FRAME_TIME (1000 / FPS)

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
chip_t *chip = NULL;

uint64_t counter = 0;

uint64_t last_frame_time;

void initialize_window(void) {
    check(SDL_Init(SDL_INIT_VIDEO) == 0, "Failed to initialize SDL.\n");

    int rc = SDL_CreateWindowAndRenderer(
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_BORDERLESS,
            &window,
            &renderer
    );
    check(rc == 0, "Failed to initialize window and renderer.\n");

    return;

error:
    exit(1);
}

void setup(void) {
    chip = create_chip();
    chip->running = true;
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            chip->running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: 
                case SDLK_q: 
                    chip->running = false;
                    break;
            }
            break;
        case SDL_KEYUP:
            break;
    }
}

void update() {
    last_frame_time = SDL_GetTicks64();

    size_t pos = counter % SCREEN_LEN;
    chip->screen[pos] = ~chip->screen[pos];
    counter++;
}

void render() { 
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect pixel = { 0, 0, SCALE_FACTOR, SCALE_FACTOR };
    uint8_t *screen_buffer = chip->screen;
    for (int i = 0; i < SCREEN_LEN; i++) {
        if (screen_buffer[i] == 0) {
            continue;
        }
        pixel.x = (int)(i % BUFFER_WIDTH) * SCALE_FACTOR;
        pixel.y = (int)(i / BUFFER_WIDTH) * SCALE_FACTOR;
        SDL_RenderFillRect(renderer, &pixel);
    }
    SDL_RenderPresent(renderer);
}

void destroy_window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

uint64_t time_to_next_frame(void) {
    uint64_t time_to_wait = TARGET_FRAME_TIME - (SDL_GetTicks64() - last_frame_time);
    return (time_to_wait > 0 && time_to_wait <= TARGET_FRAME_TIME) ? time_to_wait : 0;
}

void game_loop(void) {
    process_input();
    update();
    render();
}

int main(int argc, char *argv[]) {
    initialize_window();
    log_info("Window initialized.\n");

    setup();
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(game_loop, FPS, 1);
#else
    while (chip->running) { 
        game_loop(); 
        SDL_Delay(time_to_next_frame());
    }
#endif
    log_info("Exiting game loop.\n");

    destroy_window();

    return 0;
}
