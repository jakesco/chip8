#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <stdint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "dbg.h"

#define BUFFER_WIDTH 64
#define BUFFER_HEIGHT 32
#define SCALE_FACTOR 10

#define WINDOW_WIDTH (BUFFER_WIDTH * SCALE_FACTOR)
#define WINDOW_HEIGHT (BUFFER_HEIGHT * SCALE_FACTOR)

#define FPS 60
#define TARGET_FRAME_TIME (1000 / FPS)

bool running = false;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

struct game_object {
    float x;
    float y;
    float w;
    float h;
    float v_x;
    float v_y;
} ball;

uint64_t last_frame_time;

bool initialize_window(void) {
    check(SDL_Init(SDL_INIT_VIDEO) == 0, "Failed to initialize SDL.\n");

    int rc = SDL_CreateWindowAndRenderer(
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_BORDERLESS,
            &window,
            &renderer
    );
    check(rc == 0, "Failed to initialize window and renderer.\n");

    return true;

error:
    return false;
}

void setup(void) {
    ball.x = 20;
    ball.y = 20;
    ball.w = 15;
    ball.h = 15;
    ball.v_x = 300;
    ball.v_y = 300;
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: 
                case SDLK_q: 
                    running = false;
                    break;
            }
            break;
        case SDL_KEYUP:
            break;
    }
}

void update() {
    // Delta time factor in seconds.
    float delta_time = (SDL_GetTicks64() - last_frame_time) / 1000.0f;

    last_frame_time = SDL_GetTicks64();

    // Update ball position
    ball.x += ball.v_x * delta_time;
    ball.y += ball.v_y * delta_time;

    // Check ball collision with wall
    if (ball.x <= 0 || (ball.x + ball.w) >= WINDOW_WIDTH) {
        ball.v_x *= -1;
    }
    if (ball.y <= 0 || (ball.y + ball.h) >= WINDOW_HEIGHT) {
        ball.v_y *= -1;
    }
}

void render() { 
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw ball
    SDL_Rect ball_rect = { 
        (int)ball.x,
        (int)ball.y,
        (int)ball.w,
        (int)ball.h
    };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &ball_rect);

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
    running = initialize_window();
    log_info("Window initialized. Running: %d\n", running);

    setup();
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(game_loop, FPS, 1);
#else
    while (running) { 
        game_loop(); 
        SDL_Delay(time_to_next_frame());
    }
#endif
    log_info("Exiting game loop. Running: %d\n", running);

    destroy_window();

    return 0;
}
