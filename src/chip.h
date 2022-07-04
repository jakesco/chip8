#ifndef __CPU_H
#define __CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "dbg.h"

#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define BUFFER_WIDTH 64
#define BUFFER_HEIGHT 32
#define SCALE_FACTOR 10

#define WINDOW_WIDTH (BUFFER_WIDTH * SCALE_FACTOR)
#define WINDOW_HEIGHT (BUFFER_HEIGHT * SCALE_FACTOR)

#define FPS 60
#define TARGET_FRAME_TIME (1000 / FPS)
#define CLOCK 700
#define CLOCK_FRAME_TIME (1000 / CLOCK)

#define BUFFER_LEN (BUFFER_WIDTH * BUFFER_HEIGHT) 

typedef uint16_t address_t; 

#endif

