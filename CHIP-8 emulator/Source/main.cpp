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

    // Set up SDL
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 320;
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) { // Only use audio and sound subsystems
        std::cerr << "Error in initializing SDL subsystems.\n" << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::cerr << "Couldn't create window.\n" << SDL_GetError() << std::endl;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create texture from screen buffer
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

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

    SDL_Event event;
    // Emulation Loop
    while (chip8.pc < 4096) {
        chip8.emulateCycle();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}