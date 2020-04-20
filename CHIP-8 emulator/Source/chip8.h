#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>

class Chip8 {

    /* RAM */

    // 4096 bytes of memory
    uint8_t  memory[4096];

    /* CPU */

    // 2 byte long opcodes
    uint16_t opcode;

    // 16 byte sized registers
    // 15 general purpose registers (V0 - VE), one carry flag
    uint8_t V[16];

    // stack
    uint16_t stack[16];
    uint16_t sp;

    // index register + program counter (range of 0x000 - 0xFFF)
    // System Memory Map:
    // 0x000 - 0x1FF - Chip 8 interpreter (contains font set in emu)
    // 0x050 - 0x0A0 - Used for the built in 4x5 pixel font set(0 - F)
    // 0x200 - 0xFFF - Program ROM and work RAM
    uint16_t I, pc;

    /* Display */

    // 64 x 32 pixel res, state array (1 or 0)
    uint8_t screen[64 * 32];

    /* Sound */

    // two timer registers that count at 60hz, when set they count down to zero
    uint8_t delayTimer;
    uint8_t soundTimer;

    /* Keypad */

    // hex based keypad, each key stores a state
    uint8_t key[16];

public:

    Chip8();
    
    bool loadRom(std::string);
    
    // initialized memory and registers once
    void initialize();

    // emulates one cycle: Fetch, Decode, then Execute opcode; update timer afterwards
    void emulateCycle();

    ~Chip8();
};

#endif