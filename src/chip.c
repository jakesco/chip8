#include "chip.h"

const address_t DT   = 0x001;
const address_t ST   = 0x002;
const address_t I    = 0x003; // 16-bit
const address_t PC   = 0x005; // 16-bit
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

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
uint8_t *rom = NULL;

uint64_t counter = 0;

bool running = false;
uint8_t key_pressed = 0xFF; // Valid values 0x00 - 0x0F, 0xFF indicates no key pressed
uint8_t ram[4096] = { 0 }; // 0x000 - 0xFFF
uint8_t screen[BUFFER_LEN] = { 0 }; // 0x000 - 0x800
                            
uint8_t sp = 0; // stack pointer
address_t stack[255] = { 0 }; // address stack

uint64_t last_frame_time;

void _put_16(uint8_t *array, address_t address, uint16_t value) {
    /* Put a 16-bit value into ram starting at address */
    array[address] = value >> 8;
    array[address + 1] = value;
}

uint16_t _pull_16(uint8_t *array, address_t address) {
    /* Pull a 16-bit value out of ram starting at address */
    return (array[address] << 8) + array[address + 1];
}

void _stack_push(address_t address) {
    if (sp == 255) {
        log_err("Stack overflow.\n");
    }
    stack[sp++] = address;
}

address_t _stack_pop(void) {
    if (sp == 0) {
        log_err("Stack pointer popped when SP is 0.\n");
        return 0x000;
    }
    return stack[--sp];
}

bool setup(void) {
    check(SDL_Init(SDL_INIT_VIDEO) == 0, "Failed to initialize SDL.\n");

    int rc = SDL_CreateWindowAndRenderer(
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_BORDERLESS,
            &window,
            &renderer
    );
    check(rc == 0, "Failed to initialize window and renderer.\n");

    // Load default fonts
    size_t arr_length = sizeof DEFAULT_FONTS / sizeof DEFAULT_FONTS[0];
    for(int i = 0; i < arr_length; i++) {
        ram[FNT + i] = DEFAULT_FONTS[i];
    }

    return true;

error:
    return false;
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

void update(void) {
    last_frame_time = SDL_GetTicks64();

    size_t pos = counter % BUFFER_LEN;
    screen[pos] = ~screen[pos];
    counter++;
}

void render(void) { 
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect pixel = { 0, 0, SCALE_FACTOR, SCALE_FACTOR };
    uint8_t *screen_buffer = screen;
    for (int i = 0; i < BUFFER_LEN; i++) {
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

uint64_t time_to_next_tick(void) {
    uint64_t time_to_wait = TARGET_FRAME_TIME - (SDL_GetTicks64() - last_frame_time);
    return (time_to_wait > 0 && time_to_wait <= TARGET_FRAME_TIME) ? time_to_wait : 0;
}

void tick(void) {
    process_input();
    update();
    render();
}

int main(int argc, char *argv[]) {
    running = setup();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(tick, CLOCK, 1);
#else
    while (running) { 
        tick(); 
        SDL_Delay(time_to_next_tick());
    }
#endif
    log_info("Exiting game loop.\n");

    destroy_window();

    return EXIT_SUCCESS;
}

/* ============================================================================
 * ===================          Instructions             ======================
 * ============================================================================
 * The original implementation of the Chip-8 language includes 36 different 
 * instructions, including math,graphics, and flow control functions. All 
 * instructions are 2 bytes long and are stored most-significant-byte first. 
 * In memory, the first byte of each instruction should be located at an even 
 * addresses. If a program includes sprite data, it should be padded so any 
 * instructions following it will be properly situated in RAM.
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


void ins_sys(address_t address) {
    /*
    0nnn - SYS addr (Unimplemented)
    Jump to a machine code routine at nnn.
    */
}

void ins_clear(void) {
    /*
    00E0 - CLS
    Clear the display.
    */
    uint8_t *vram = screen;
    for(int i = 0; i < BUFFER_LEN; i++) {
        vram[i] = 0;
    }
    debug("Cleared Screen.");
}

void ins_return(void) {
    /*
    00EE - RET
    Return from a subroutine.
    */
    address_t address = _stack_pop();
    ram[PC] = address;
    debug("Return from subroutine. Set PC to @0x%x.\n", address);
}

void ins_jump(address_t address) {
    /*
    1nnn - JP addr
    Jump to location nnn.
    */
    ram[PC] = address;
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

*/

void ins_set(uint8_t x, uint8_t byte) {
    /* 
    6xkk - LD Vx, byte
    Set Vx = kk.
    */ 
    ram[GP + x] = byte;
    debug("Set 'Register' V%X to 0x%x.", x, byte);
}

void ins_add(uint8_t x, uint8_t byte) {
    /* 
    7xkk - ADD Vx, byte
    Set Vx = Vx + kk.
    */ 
    ram[GP + x] += byte;
    debug("Add 0x%x to register V%X.", byte, x);
}

/*
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
*/

void ins_set_i(address_t address) {
    /*
    Annn - LD I, addr
    Set I = nnn.
    */
    _put_16(ram, I, address);
    debug("Set register I to 0x%x.", address);
}

/*
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
*/

void ins_display(uint8_t x, uint8_t y, size_t sprite_size) {
    /*
    Dxyn - DRW Vx, Vy, nibble
    Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
    */
    uint8_t x_start = ram[GP + x] % 64;
    uint8_t y_start = ram[GP + y] % 32;
    int max_y = fmin(32 - y_start, sprite_size);
    int max_x = fmin(64 - x_start, 8);
    address_t sprite_idx = _pull_16(ram, I);

    debug("Draw: (%d, %d) | %d %d | %d\n", x_start, y_start, max_x, max_y, sprite_idx);
}

/*
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
