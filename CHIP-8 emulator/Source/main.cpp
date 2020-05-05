#include "chip8.hpp"
#include <iostream>
#include <SDL.h>

// Display size
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

Chip8 myChip8;
int modifier = 10;

// Window size
int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;



int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        std::cerr << ("Path to executable and ROM must be given as arguments.\n");
        exit(1);
    }

    return 0;
}