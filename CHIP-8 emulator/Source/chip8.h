#ifndef CHIP8_H
#define CHIP8_H

class Chip8 {

    /* RAM */
    // 4096 bytes of memory
    unsigned char  memory[4096];

    /* CPU */
    // 2 byte long opcodes
    unsigned short opcode;
    // 16 byte sized registers
    unsigned char reg[16];
    // stack
    unsigned short stack[16];
    unsigned short sp;
    // index register + program counter
    // 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    // 0x050 - 0x0A0 - Used for the built in 4x5 pixel font set(0 - F)
    // 0x200 - 0xFFF - Program ROM and work RAM
    unsigned short I, pc;

    /* Display */
    // 64 x 32 pixel res
    int screen[64 * 32]; // could use a vector or multi-d array for this

    /* Sound */
    

public:

    ~Chip8();
};

#endif