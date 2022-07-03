#ifndef __CPU_H
#define __CPU_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "dbg.h"

#define SCREEN_LEN (64 * 32)

typedef uint16_t address_t; 

typedef struct {
    bool running;
    uint8_t clock; // ticks per second
    uint8_t key_pressed; // Valid values 0x00 - 0x0F, 0xFF indicates no key pressed
    uint8_t sp; // stack pointer
    uint8_t *last_rom; // pointer to original rom for reset
    uint8_t ram[4096]; // 0x000 - 0xFFF
    uint8_t screen[SCREEN_LEN]; // 0x000 - 0x800
    address_t stack[255]; // address stack
} chip_t;

chip_t *chip_create(void);
void chip_destroy(chip_t *chip_ptr);
bool chip_load_rom(chip_t *chip_ptr, const uint8_t *rom);
bool chip_halt(void);
bool chip_start(void);
bool chip_reset(void);


#endif

