#include "chip8.hpp"
#include <iostream>
#include <SDL.h>

int main(int argc, char *argv[]) {  
    if (argc < 2) {
        std::cerr << ("Path to executable and ROM must be given as arguments.\n");
        exit(1);
    }

    // Initialize system
    Chip8 chip8;

    // Load game
    if(!chip8.loadRom(argv[1])) { 
        std::cerr << "ROM not given as an argument.\n";
        return 1;
    }

    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 320;
    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
        );

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create texture from screen buffer
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    // Keypad map
    uint8_t keyMap[16]{
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_4,
        SDLK_q,
        SDLK_w,
        SDLK_e,
        SDLK_r,
        SDLK_a,
        SDLK_s,
        SDLK_d,
        SDLK_f,
        SDLK_z,
        SDLK_x,
        SDLK_c,
        SDLK_v
    };

    uint32_t pixels[2048]; // Temporary pixel buffer
    SDL_Event event;       // Event used to handle keypresses
    // Emulation Loop
    while(true) {
        chip8.emulateCycle();

        // Event handling //
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) { return 0; }

            // Process when key is held down
            if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.sym == SDLK_ESCAPE) {
                    return 0;
                }

                for(int i = 0; i < 16; i++) {
                    if(event.key.keysym.sym == keyMap[i]) {
                        chip8.key[i] = 1;
                    }
                }
            }

            if(event.type == SDL_KEYUP) {
                for(int i = 0; i < 16; i++) {
                    if(event.key.keysym.sym == keyMap[i]) {
                        chip8.key[i] = 0;
                    }
                }
            }
        }
        // End of event //

        if (chip8.drawFlag) {
            chip8.drawFlag = false;
            uint32_t baseColor = 0xFFFFFFFF; // white

            for (int i = 0; i < 2048; i++) {
                if (chip8.screen[i] == 0) {
                    pixels[i] = 0xFF000000; // black
                    baseColor -= 8191;      // sky blue gradient
                } else {
                    std::cout << "I run!"; // make this shit run
                    pixels[i] = baseColor;
                    baseColor -= 8191;
                }
            }

            // Update texture
            SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
            // Clear screen and renderer
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        SDL_Delay(16); // 1000 ms / 60 instructions per second ~= 16 ms delay
    }

    return 0;
}