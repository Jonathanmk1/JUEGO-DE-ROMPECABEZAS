// game_bridge.cpp — Puente que llama a la implementación C++ del juego.
#include "game_bridge.h"

// Implementación verdadera en tu src/main_sdl_kinect.cpp (ver cambios abajo).
// La declaramos aquí para enlazar.
extern int run_puzzle_game(const char* image_path, int grid, int size,
                           SDL_Window* existing_window, SDL_Renderer* existing_renderer);

extern "C" int run_puzzle_game_c(const char* image_path, int grid, int size,
                                 SDL_Window* existing_window, SDL_Renderer* existing_renderer) {
    return run_puzzle_game(image_path, grid, size, existing_window, existing_renderer);
}
