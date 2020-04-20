#include "chip8.h" // CPU implementation
#include <cstring>
#include <fstream>
#include <time.h>

Chip8::Chip8() { // Prepare system state before emulation cycles

    pc     = 0x200;  // Program counter starts at 0x200
    opcode = 0;      // Reset current opcode	
    I      = 0;      // Reset index register
    sp     = 0;      // Reset stack pointer
    
    // Clear display
    for(int i = 0; i < 2048; i++) {
        screen[i] = 0;
    }
    // Clear stack
    for(int i = 0; i < 16; i++) {
        stack[i] = 0;
    }
    // Clear registers V0-VF
    for (int i = 0; i < 16; i++) {
        V[i] = 0;
    }
    // Clear memory
    for (int i = 0; i < 4096; i++) {
        memory[i] = 0;
    }

    unsigned char chip8Fontset[80] = {
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
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    // Load fontset
    for (int i = 0; i < 80; i++) {
        memory[i] = chip8Fontset[i];
    }

    // Reset timers
    delayTimer = 0;
    soundTimer = 0;

    // Clear screen
    drawFlag = true;

    srand(time(NULL)); // Seed for opcode
}

/* Member Methods */

// Emulates one cycle: Fetch, Decode, then Execute opcode; update timer afterwards
void Chip8::emulateCycle() {
    // Fetch 2 byte opcode
    unsigned short opcode = (memory[pc] << 8) | memory[pc + 1];
    
    // Decode opcode

    // Execute opcode

    // Update timers
}

Chip8::~Chip8() {}