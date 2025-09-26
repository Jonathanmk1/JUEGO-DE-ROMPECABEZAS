// game_bridge.h — Puente C <-> C++ para ejecutar el juego en la misma ventana.
#pragma once
#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

// Ejecuta el rompecabezas dentro de la MISMA ventana/renderer.
// Bloquea hasta que el juego termina (retorna 0 si ok).
int run_puzzle_game_c(const char* image_path, int grid, int size,
                      SDL_Window* existing_window, SDL_Renderer* existing_renderer);

#ifdef __cplusplus
}
#endif
