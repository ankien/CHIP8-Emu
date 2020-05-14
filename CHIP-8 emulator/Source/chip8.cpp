#include "chip8.hpp" // CPU implementation
#include <time.h>
#include <iostream>
#include <fstream>
#include <math.h>

// Initialize SDL audio
uint16_t audioBuffer[20480]; // buffer size in bytes
SDL_AudioDeviceID audioDevice;
SDL_AudioSpec audioSpec;

Chip8::Chip8() { // Prepare system state before emulation cycles
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Couldn't init audio! Continuing..." << std::endl;
    }
    audioSpec.freq = 44100; // 44.1 khz sample rate
    audioSpec.format = AUDIO_S16SYS;
    audioSpec.channels = 1;
    audioSpec.samples = 2048;
    audioSpec.callback = NULL;

    audioDevice = SDL_OpenAudioDevice(NULL, 0, &audioSpec, NULL, 0);

    float x = 0;
    for(int i = 0; i < 20480; ++i, x += 0.001) {
        audioBuffer[i] = sin(x) * 20000; // amplitude
    }

    SDL_PauseAudioDevice(audioDevice, 0); // start audio


    pc     = 0x200;  // Program counter starts at 0x200
    opcode = 0;      // Reset current opcode	
    I      = 0;      // Reset index register
    sp     = 0;      // Reset stack pointer
    
    // Clear display memory
    for(int i = 0; i < 2048; i++) {
        screen[i] = 0;
    }
    // Clear stack
    for(int i = 0; i < 16; i++) {
        stack[i] = 0;
    }
    // Clear registers V0-VF and keypad
    for (int i = 0; i < 16; i++) {
        key[i] = V[i] = 0;
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

    srand(time(NULL)); // Seed for random opcode(s)
}

/* Member Methods */

// Emulates one cycle: Fetch, Decode, then Execute opcode; update timer afterwards
void Chip8::emulateCycle() {
    // Fetch 2 byte opcode
    opcode = (memory[pc] << 8) | memory[pc + 1];
    
    // Decode & execute opcode
    switch (opcode & 0xF000) { // huge ass switch-case may not be the most efficient interpreter method
        case 0x0000:
            switch (opcode & 0x000F) {
                case 0x0000: // 0x00E0: Clear the screen
                    for (int i = 0; i < 2048; i++) {
                        screen[i] = 0;
                    }
                    drawFlag = false;
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
            drawFlag = true;
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

                case 0x0001: // 0x8XY1: Set V[X] to V[X] OR V[Y]
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0002: // 0x8XY2: Set V[X] to V[X] AND V[Y]
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0003: // 0x8XY3: Set V[X] to V[X] XOR V[Y]
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

        case 0xB000: // 0xBNNN: Jump to address NNN + V[0]
            pc = (opcode & 0x0FFF) + V[0];
        break;

        case 0xC000: // 0xCXNN: Set V[X] to the result of a AND on a random number (0-255) and NN
            V[(opcode & 0x0F00) >> 8] = (rand() & 255) & (opcode & 0x00FF);
            pc += 2;
        break;

        case 0xD000: // 0xDXYN: Draw a sprite at coordinate (V[X],V[Y]) that has a width of 8 pixels and height of N pixels
                     //         Each row of 8 pixels is read as bit-coded starting from memory location I; 
                     //         I value doesn't change after the execution of this instruction. 
                     //         V[F] is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn (collision detection), 
                     //         and to 0 if that doesn't happen
        {
            uint8_t x = V[(opcode & 0x0F00) >> 8];
            uint8_t y = V[(opcode & 0x00F0) >> 4];
            uint8_t height = opcode & 0xF;
            uint8_t spriteRow;
            
            V[0xF] = 0;

            for(int yLine = 0; yLine < height; yLine++) {
                spriteRow = memory[I + yLine];
                for (int xLine = 0; xLine < 8; xLine++) {
                    if((spriteRow & (0x80 >> xLine)) != 0) {
                        if(screen[(x + xLine + ((y + yLine) * 64)) % (2048)] == 1) {
                            V[0xF] = 1;
                        } screen[(x + xLine + ((y + yLine) * 64)) % (2048)] ^= 1;
                    }
                }
            }

            pc += 2;
        }
        break;

        case 0xE000:
            switch(opcode & 0x00FF) {
                case 0x009E: // 0xEX9E: Skip the next instruction if the key stored in V[X] is pressed
                    if (key[V[(opcode & 0x0F00) >> 8]] != 0) {
                        pc += 4;
                    } else { pc += 2; }
                break;

                case 0x00A1: // 0xEXA1: Skip the next instruction if the key stored in V[X] isn't pressed
                    if (key[V[(opcode & 0x0F00) >> 8]] == 0) {
                        pc += 4;
                    } else { pc += 2; }
                break;

                default:
                    std::cerr << "Unknown opcode [0xE000]: 0x" << std::hex << opcode << std::endl;
            }                
        break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: // 0xFX07: Set V[X] to the value of the delay timer
                    V[(opcode & 0x0F00) >> 8] = delayTimer;
                    pc += 2;
                break;

                case 0x000A: // 0xFX0A: A key press is awaited, then stored in V[X]
                {
                    bool keyPressed = false;
                    
                    for (int i = 0; i < 16; i++) {
                        if (key[i] != 0) {
                            V[(opcode & 0x0F00) >> 8] = i;
                            keyPressed = true;
                        }
                    }

                    // If key isn't pressed skip current cycle and try again
                    if(!keyPressed) { return; }

                    pc += 2;
                }
                break;

                case 0x0015: // 0xFX15: Set delay timer to V[X]
                    delayTimer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

                case 0x0018: // 0xFX18: Set sound timer to V[X]
                    soundTimer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

                case 0x001E: // 0xFX1E: Adds V[X] to I, V[F] is set to 1 when there is range overflow (I + V[X] > 0xFFF), and 0 if not
                    if ((I + V[(opcode & 0x0F00) >> 8]) > 0xFFF) {
                        V[0xF] = 1;
                    } else { V[0xF] = 0; }
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

                case 0x0029: // 0xFX29: Set I to the location of the sprite for the character in V[X], 4x5 font
                    I = V[(opcode & 0x0F00) >> 8] * 5;
                    pc += 2;
                break;

                case 0x0033: // 0xFX33: Store the BCD version of V[X] in memory at I(hundreds), I + 1(middle digit), and I + 2(rightmost digit)
                    memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
                    pc += 2;
                break;

                case 0x0055: // 0xFX55: Store V[0] - V[X] (including V[X]) in memory starting at address I. 
                             //         The offset from I is increased by 1 for each value written, but I itself is left unmodified.
                             //         Original CHIP-8 & CHIP-48 increments by X + 1 afterwards
                    for (int offset = 0; offset <= ((opcode & 0x0F00) >> 8); offset++) {
                        memory[I + offset] = V[offset];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                break;

                case 0x065: // 0xFX65: Load into V[0] - V[X] from memory starting at address I, increment afterwards
                    for (int offset = 0; offset <= ((opcode & 0x0F00) >> 8); offset++) {
                        V[offset] = memory[I + offset];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                break;

                default:
                    std::cerr << "Unknown opcode [0xF000]: 0x" << std::hex << opcode << std::endl;
            }
        break;

        default:
            std::cerr << "Unknown opcode: 0x" << std::hex << opcode << std::endl;
    }
    if(delayTimer > 0) { 
        delayTimer--;

    }
    if(soundTimer > 0) {
        if (soundTimer == 1) {
            SDL_QueueAudio(audioDevice, audioBuffer, soundTimer * 20480);
        }
        soundTimer--;
    }
}

bool Chip8::loadRom(std::string romName) {
    std::fstream fileStream(romName, std::ios::binary | std::ios::in);

    if(!fileStream.is_open()) { return false; }

    char byte;
    for (int i = 0x200; fileStream.get(byte); i++) {
        if(i >= 4096) { return false; }
        memory[i] = (uint8_t) byte;
    }
    return true;
}