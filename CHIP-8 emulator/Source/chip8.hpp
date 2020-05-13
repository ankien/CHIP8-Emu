#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <string>
#include <SDL.h>

class Chip8 {
    uint8_t  memory[4096]; // 4096 bytes of memory

    uint16_t opcode; // 2 byte long opcodes

    // 16 byte sized registers
    // 15 general purpose registers (V0 - VE), one carry flag
    uint8_t V[16];

    // Stack
    uint16_t stack[16];
    uint16_t sp;

    // Index register + program counter (range of 0x000 - 0xFFF)
    // System Memory Map:
    // 0x000 - 0x1FF - Chip 8 interpreter (contains font set in emu)
    // 0x050 - 0x0A0 - Used for the built in 4x5 pixel font set(0 - F)
    // 0x200 - 0xFFF - Program ROM and work RAM
    uint16_t I;

    // Two timer registers that count at 60hz, when set they count down to zero
    uint8_t delayTimer;
    uint8_t soundTimer;

public:
    Chip8();
    
    bool loadRom(std::string);

    uint16_t pc;
    
    uint8_t screen[2048]; // 64 x 32 (2048) pixel res, state array for display memory (1 or 0)
    bool drawFlag; // Need to draw if true
    
    uint8_t key[16]; // Hex based keypad, each key stores a state

    void emulateCycle(); // Emulates one cycle: Fetch, Decode, then Execute opcode; update timer afterwards
};

#endif