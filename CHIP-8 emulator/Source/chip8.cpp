#include "chip8.h" // CPU implementation
#include <time.h>
#include <iostream>

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
    
    // Decode & execute opcode
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F) {
                case 0x0000: // 0x00E0: Clear the screen
                    for (int i = 0; i < 2048; i++) {
                        screen[i] = 0x0;
                    }
                    drawFlag = true;
                    pc += 2;
                break;
                    
                case 0x000E: // 0x00EE: Returns from subroutine
                    --sp;
                    pc = stack[sp];
                    pc += 2;
                break;

                default:
                    std::cerr << "Unknown opcode [0x0000]: 0x" << std::hex << opcode << std::endl;
            }
        break;

        case 0x1000: // 0x1NNN: Jump to address NNN
            pc = opcode & 0x0FFF;
        break;

        case 0x2000: // 0x2NNN: Call subroutine at NNN
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
        break;

        case 0x3000: // 0x3XNN: Skip the next instruction if register V[X] == NN
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0x4000: // 0x4XNN: Skip the next instruction if V[X] != NN
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0x5000: // 0x5XY0: Skip the next instruction if V[X] == V[Y]
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0x6000: // 0x6XNN: Set V[X] = NN
            V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            pc += 2;
        break;

        case 0x7000: // 0x7XNN: Add NN to V[X]
            V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
            pc += 2;
        break;

        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000: // 0x8XY0: Set V[X] to V[Y]
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0001: // 0x8XY1: Set V[X] to V[X] or V[Y]
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0002: // 0x8XY2: Set V[X] to V[X] and V[Y]
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0003: // 0x8XY3: Set V[X] to V[X] xor V[Y]
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0004: // 0x8XY4: Adds V[Y] to V[X]; V[F] is set to 1 if there's a carry, 0 if there isn't
                    if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0005: // 0x8XY5: Subtracts V[Y] from V[X]; V[F] is set to 0 if there's a borrow, 1 if there isn't
                    if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0006: // 0x8XY6: Store the rightmost bit of V[X] into V[F] and shift V[X] to the right by 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                break;

                case 0x0007: // 0x8XY7: Set V[X] to V[Y] - V[X]; V[F] is set to 0 if there's a borrow, 1 if there isn't
                    if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

                case 0x000E: // 0x8XYE: Store the leftmost bit of V[X] into V[F] and shift V[X] to the left by 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                break;

                default:
                    std::cerr << "Unknown opcode [0x8000]: 0x" << std::hex << opcode << std::endl;
            }
        break;

        case 0x9000: // 0x9XY0: Skip next instruction if V[X] != V[Y]
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0xA000: // 0xANNN: Sets I to the address NNN
            I = opcode & 0x0FFF;
            pc += 2;
        break;

        case 0xB000:
        break;

        default:
            std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::endl;
    }

    // Update timers
}

Chip8::~Chip8() {}