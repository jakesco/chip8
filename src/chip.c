#include "chip.h"

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

const address_t DT   = 0x001;
const address_t ST   = 0x002;
const address_t I    = 0x003;
const address_t PC   = 0x005;
const address_t GP   = 0x010;
const address_t FNT  = 0x050;
const address_t PROG = 0x200;

const uint8_t DEFAULT_FONTS[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

chip_t *create_chip(void) {
    chip_t *chip_ptr = calloc(1, sizeof(chip_t));
    check_mem(chip_ptr);

    // Load default fonts
    size_t arr_length = sizeof DEFAULT_FONTS / sizeof DEFAULT_FONTS[0];
    for(int i = 0; i < arr_length; i++) {
        chip_ptr->ram[FNT + i] = DEFAULT_FONTS[i];
    }

    return chip_ptr;

error:
    exit(1);
}

void destroy_chip(chip_t *chip_ptr) {
    free(chip_ptr);
}

/* =============================================================================================================
 * =====================                      Instructions                                ======================
 * =============================================================================================================
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


void ins_sys(chip_t *chip, address_t address) {
    /*
    0nnn - SYS addr (Unimplemented)
    Jump to a machine code routine at nnn.
    */
}

void ins_clear(chip_t *chip) {
    /*
    00E0 - CLS
    Clear the display.
    */
    uint8_t *vram = chip->screen;
    for(int i = 0; i < SCREEN_LEN; i++) {
        vram[i] = 0;
    }
    debug("Cleared Screen.");
}

void ins_return(chip_t *chip) {
    /*
    00EE - RET
    Return from a subroutine.
    */
    address_t address = chip->stack[chip->sp--];
    chip->ram[PC] = address;
    debug("Return from subroutine. Set PC to @0x%x.\n", address);
}

void ins_jump(chip_t *chip, address_t address) {
    /*
    1nnn - JP addr
    Jump to location nnn.
    */
    chip->ram[PC] = address;
    debug("Jump to: 0x%03x.", address);
}


/*
def call(chip: Chip8, address: Address):
    """
    2nnn - CALL addr
    Call subroutine at nnn.
    """
    chip.stack_push(chip.pc)
    chip.set_pc(address)


def skip_eq(chip: Chip8, x: Register, byte: int):
    """
    3xkk - SE Vx, byte
    Skip next instruction if Vx = kk.
    """
    if chip.get_register(x) == byte:
        chip.set_pc(chip.pc + 2)


def skip_neq(chip: Chip8, x: Register, byte: int):
    """
    4xkk - SNE Vx, byte
    Skip next instruction if Vx != kk.
    """
    if chip.get_register(x) != byte:
        chip.set_pc(chip.pc + 2)


def skip_eqr(chip: Chip8, x: Register, y: Register):
    """
    5xy0 - SE Vx, Vy
    Skip next instruction if Vx = Vy.
    """
    if chip.get_register(x) == chip.get_register(y):
        chip.set_pc(chip.pc + 2)


def set_(chip: Chip8, register: Register, byte: int):
    """
    6xkk - LD Vx, byte
    Set Vx = kk.
    """
    logger.info("Set 'Register' V%d to 0x%02x.", register, byte)
    chip.set_register(register, byte)


def add(chip: Chip8, register: Register, byte: int):
    """
    7xkk - ADD Vx, byte
    Set Vx = Vx + kk.
    """
    logger.info("Add 0x%02x to register V%d.", byte, register)
    chip.set_register(register, chip.get_register(register) + byte)


def set_r(chip: Chip8, x: Register, y: Register):
    """
    8xy0 - LD Vx, Vy
    Set Vx = Vy.
    """
    val = chip.get_register(y)
    logger.info("Setting register V%01x to 0x%02x.", x, val)
    chip.set_register(x, val)


def or_(chip: Chip8, x: Register, y: Register):
    """
    8xy1 - OR Vx, Vy
    Set Vx = Vx OR Vy.
    """
    logger.info("Setting register V%01x to V%01x or V%01x.", x, y, x)
    val = chip.get_register(x) | chip.get_register(y)
    chip.set_register(x, val)


def and_(chip: Chip8, x: Register, y: Register):
    """
    8xy2 - AND Vx, Vy
    Set Vx = Vx AND Vy.
    """
    logger.info("Setting register V%01x to V%01x and V%01x.", x, y, x)
    val = chip.get_register(x) & chip.get_register(y)
    chip.set_register(x, val)


def xor_(chip: Chip8, x: Register, y: Register):
    """
    8xy3 - XOR Vx, Vy
    Set Vx = Vx XOR Vy.
    """
    logger.info("Setting register V%01x to V%01x xor V%01x.", x, y, x)
    val = chip.get_register(x) ^ chip.get_register(y)
    chip.set_register(x, val)


def add_r(chip: Chip8, x: Register, y: Register):
    """
    8xy4 - ADD Vx, Vy
    Set Vx = Vx + Vy, set VF = carry.
    """
    logger.info("Add registers V%01x and V%01x into V%01x.", x, y, x)
    chip.clear_vf()
    val = chip.get_register(x) + chip.get_register(y)
    if val > 0xFF:
        chip.set_vf()
    chip.set_register(x, val)


def sub_xy(chip: Chip8, x: Register, y: Register):
    """
    8xy5 - SUB Vx, Vy
    Set Vx = Vx - Vy, set VF = NOT borrow.
    """
    logger.info("Subtract register V%01x from V%01x into V%01x.", y, x, x)
    chip.clear_vf()
    val = chip.get_register(x) - chip.get_register(y)
    if val >= 0x00:
        chip.set_vf()
    chip.set_register(x, val)


def shift_r(chip: Chip8, x: Register, y: Register = None):
    """
    8xy6 - SHR Vx {, Vy}
    Set Vx = Vx SHR 1.
    Some implementations set Vx to value in Vy before shift.
    I've chosen to make it optional. Vx will be set to Vy
    if Vy is provided, otherwise Vx will be shifted in place.
    """
    logger.info("Shift V%01x right.", x)
    chip.clear_vf()
    if y is not None:
        chip.set_register(x, chip.get_register(y))
    val = chip.get_register(x)
    if val & 0x01 > 0:
        chip.set_vf()
    chip.set_register(x, val >> 1)


def sub_yx(chip: Chip8, x: Register, y: Register):
    """
    8xy7 - SUBN Vx, Vy
    Set Vx = Vy - Vx, set VF = NOT borrow.
    """
    logger.info("Subtract register V%01x from V%01x into V%01x.", x, y, x)
    chip.clear_vf()
    val = chip.get_register(y) - chip.get_register(x)
    if val >= 0x00:
        chip.set_vf()
    chip.set_register(x, val)


def shift_l(chip: Chip8, x: Register, y: Register = None):
    """
    8xyE - SHL Vx {, Vy}
    Set Vx = Vx SHL 1.
    Some implementations set Vx to value in Vy before shift.
    I've chosen to make it optional. Vx will be set to Vy
    if Vy is provided, otherwise Vx will be shifted in place.
    """
    logger.info("Shift V%01x left.", x)
    chip.clear_vf()
    if y is not None:
        chip.set_register(x, chip.get_register(y))
    val = chip.get_register(x)
    if val & 0x80 > 0:
        chip.set_vf()
    chip.set_register(x, val << 1)


def skip_ner(chip: Chip8, x: Register, y: Register):
    """
    9xy0 - SNE Vx, Vy
    Skip next instruction if Vx != Vy.
    """
    if chip.get_register(x) != chip.get_register(y):
        chip.set_pc(chip.pc + 2)


def set_index(chip: Chip8, address: Address):
    """
    Annn - LD I, addr
    Set I = nnn.
    """
    logger.info("Set register I to 0x%03x.", address)
    chip.set_i(address)


def jump_with_offset(chip: Chip8, address: Address):
    """
    Bnnn - JP V0, addr
    Jump to location nnn + V0.
    """
    new_addr = address + chip.get_register(0)
    logger.info("Jump to: 0x%03x.", new_addr)
    chip.set_pc(new_addr)


def random_(chip: Chip8, x: Register, byte: int):
    """
    Cxkk - RND Vx, byte
    Set Vx = random byte AND kk.
    """
    logger.info("Generating random integer.")
    val = random.randint(0x00, 0xFF) & byte
    chip.set_register(x, val)


def display(chip: Chip8, x: Register, y: Register, sprite_size: int):
    """
    Dxyn - DRW Vx, Vy, nibble
    Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
    """
    x_start = chip.get_register(x) % 64
    y_start = chip.get_register(y) % 32
    max_y = min(32 - y_start, sprite_size)
    max_x = min(64 - x_start, 8)
    sprite_idx = chip.i

    logger.info(
        "Draw %d byte sprite at memory address 0x%03x starting at coordinate (%d, %d).",
        sprite_size,
        chip.i,
        x_start,
        y_start,
    )

    chip.clear_vf()
    for offset in range(max_y):
        bits = unpack_8bit_hex(chip.ram[sprite_idx + offset])
        start_addr = (64 * (y_start + offset)) + x_start
        for idx in range(max_x):
            addr = start_addr + idx
            new_bit = bits[idx]
            old_bit = chip.vram[addr]
            set_bit = new_bit ^ old_bit
            logger.debug("Setting VRAM @0x%03x to 0x%02x.", addr, set_bit)
            chip.vram[addr] = set_bit
            if (not chip.vf) and (new_bit & old_bit):
                logger.debug("Collision detected, setting VF.")
                chip.set_vf()


def skip_if_pressed(chip: Chip8, x: Register):
    """
    Ex9E - SKP Vx
    Skip next instruction if key with the value of Vx is pressed.
    """
    val = chip.get_register(x)
    if chip.key_pressed == val:
        chip.set_pc(chip.pc + 2)


def skip_if_not_pressed(chip: Chip8, x: Register):
    """
    ExA1 - SKNP Vx
    Skip next instruction if key with the value of Vx is not pressed.
    """
    val = chip.get_register(x)
    if chip.key_pressed != val:
        chip.set_pc(chip.pc + 2)


def load_dt(chip: Chip8, x: Register):
    """
    Fx07 - LD Vx, DT
    Set Vx = delay timer value.
    """
    logger.info("Setting register V%01x to DT value of 0x%02x.", x, chip.dt)
    chip.set_register(x, chip.dt)


def wait_for_key(chip: Chip8, x: Register):
    """
    Fx0A - LD Vx, K
    Wait for a key press, store the value of the key in Vx.
    """
    if chip.key_pressed is None:
        chip.set_pc(chip.pc - 2)
    else:
        chip.set_register(x, chip.key_pressed)


def set_delay_timer(chip: Chip8, x: Register):
    """
    Fx15 - LD DT, Vx
    Set delay timer = Vx.
    """
    val = chip.get_register(x)
    logger.info("Setting delay timer to 0x%02x.", val)
    chip.set_dt(val)


def set_sound_timer(chip: Chip8, x: Register):
    """
    Fx18 - LD ST, Vx
    Set sound timer = Vx.
    """
    val = chip.get_register(x)
    logger.info("Setting sound timer to 0x%02x.", val)
    chip.set_st(val)


def add_register_to_index(chip: Chip8, x: Register):
    """
    Fx1E - ADD I, Vx
    Set I = I + Vx, set VF if I >= 0x1000.
    """
    chip.clear_vf()
    val = chip.get_register(x) + chip.i
    logger.info("Setting index to 0x%02x.", val)
    if val > 0xFF:
        chip.set_vf()
    chip.set_i(val)


def load_font_character(chip: Chip8, x: Register):
    """
    Fx29 - LD F, Vx
    Set I = location of sprite for digit Vx.
    """
    logger.info("Pointing index to built-in font %01x.", x)
    font = chip.get_register(x)
    chip.set_i(0x050 + (font * 5))


def store_bcd(chip: Chip8, x: Register):
    """
    Fx33 - LD B, Vx
    Store BCD representation of Vx in memory locations I, I+1, and I+2.
    """
    logger.info("Calculating BCD of register V%01x.", x)
    val = chip.get_register(x)
    a = val // 100
    b = val // 10 % 10
    c = val % 10
    for idx, bcd in enumerate((a, b, c)):
        chip.ram[chip.i + idx] = bcd


def store_registers_in_memory(chip: Chip8, x: Register):
    """
    Fx55 - LD [I], Vx
    Store registers V0 through Vx in memory starting at location I.
    NOTE: Older implementations incremented I as these were stored
    may want to make a config option.
    """
    logger.info("Saving registers V%01x - V%01x to @0x%03x.", 0, x, chip.i)
    for i in range(x + 1):
        chip.ram[chip.i + i] = chip.get_register(i)


def fetch_registers_from_memory(chip: Chip8, x: Register):
    """
    Fx65 - LD Vx, [I]
    Read registers V0 through Vx from memory starting at location I.
    NOTE: Older implementations incremented I as these were stored
    may want to make a config option.
    """
    logger.info("Loading registers V%01x - V%01x from @0x%03x.", 0, x, chip.i)
    for i in range(x + 1):
        chip.set_register(i, chip.ram[chip.i + i])
*/
