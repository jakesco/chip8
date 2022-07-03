#ifndef __CPU_H
#define __CPU_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "dbg.h"

/*
 *  Chip-8 Emulator ram space
 *  0x000 -> Null
 *  0x001 -> DT
 *  0x002 -> ST
 *  0x003 - 0x004 -> I
 *  0x005 - 0x006 -> PC
 *  0x010 - 0x01F -> GP registers (V0 - VF)
 *  0x050 - 0x1FF -> Reserved for Fonts
 *  0x200 - 0xFFF -> Program Space
 */

#define SCREEN_LEN (64 * 32)

typedef uint16_t address_t; 

typedef struct {
    bool running;
    uint8_t clock; // ticks per second
    uint8_t key_pressed; // Valid values 0x00 - 0x0F, 0xFF indicates no key pressed
    uint8_t sp; // stack pointer
    uint8_t ram[4096]; // 0x000 - 0xFFF
    uint8_t screen[SCREEN_LEN]; // 0x000 - 0x800
    address_t stack[255]; // address stack
} chip_t;

chip_t *create_chip(void);
void destroy_chip(chip_t *chip_ptr);
void chip_stack_push(chip_t *chip_ptr, address_t address);
address_t chip_stack_pop(chip_t *chip_ptr);



#endif

