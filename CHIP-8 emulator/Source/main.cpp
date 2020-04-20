#include "chip8.h"
#include <iostream>
#include <SDL.h>

/* uint8_t key[16]{

} */

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        std::cerr << ("Path to executable and ROM must be given as arguments.\n");
        exit(1);
    }

    return 0;
}