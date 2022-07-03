/* 
 * The original implementation of the Chip-8 language includes 36 different instructions, including math,
 * graphics, and flow control functions. All instructions are 2 bytes long and are stored most-significant-byte
 * first. In memory, the first byte of each instruction should be located at an even addresses. If a program
 * includes sprite data, it should be padded so any instructions following it will be properly situated in RAM.
 * In these listings, the following variables are used:
 * nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
 * n or nibble - A 4-bit value, the lowest 4 bits of the instruction
 * x - A 4-bit value, the lower 4 bits of the high byte of the instruction
 * y - A 4-bit value, the upper 4 bits of the low byte of the instruction
 * kk or byte - An 8-bit value, the lowest 8 bits of the instruction
 */

/* Start with these and IBM Logo ROM */
/*     00E0 (clear screen) */
/*     1NNN (jump) */
/*     6XNN (set register VX) */
/*     7XNN (add value to register VX) */
/*     ANNN (set index register I) */
/*     DXYN (display/draw) */

#include "cpu.h"

void sys(chip_t *chip, address_t address) {
    /*
    0nnn - SYS addr (Unimplemented)
    Jump to a machine code routine at nnn.
    */
}

void clear(chip_t *chip) {
    /*
    00E0 - CLS
    Clear the display.
    */
    log_info("Clear Screen.");
    uint8_t *vram = chip->screen;
    for(int i = 0; i < SCREEN_LEN; i++) {
        vram[i] = 0;
    }
}

void fill(chip_t *chip) {
    log_info("Screen filled.");
    uint8_t *vram = chip->screen;
    for(int i = 0; i < SCREEN_LEN; i++) {
        vram[i] = 1;
    }
}

