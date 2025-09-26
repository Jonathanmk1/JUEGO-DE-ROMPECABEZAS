#ifndef SDL_SLIDING_PUZZLE_H
#define SDL_SLIDING_PUZZLE_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Configuración de la aplicación
typedef struct {
    int grid;               // tamaño del grid (3 o 4)
    int win_size;           // tamaño de ventana
    char image_path[512];   // ruta de imagen
    bool step_mode;         // modo paso a paso
} AppConfig;

// Estado del puzzle
typedef struct {
    int grid;
    int empty_r, empty_c;   // posición de la celda vacía
    int *board;             // estado actual del tablero
    int *goal;              // estado objetivo
    int moves;              // número de movimientos
    Uint64 start_ticks;     // tiempo inicio
    Uint64 solved_ticks;    // tiempo cuando se resolvió
    bool paused;            // en pausa
    bool solved;            // resuelto
} Puzzle;

// Recursos gráficos
typedef struct {
    SDL_Window *win;
    SDL_Renderer *ren;
    SDL_Texture **tiles;    // texturas de cada tile
    int tile_px;            // tamaño en píxeles de cada tile
} Gfx;

// --- Funciones principales disponibles en sdl_sliding_puzzle.c ---

// Inicializa el puzzle (estructura lógica)
void puzzle_init(Puzzle *pz, int grid);

// Mezcla aleatoriamente las piezas del puzzle
void puzzle_shuffle(Puzzle *pz);

// Intenta mover una pieza en dirección (dr, dc)
bool puzzle_move(Puzzle *pz, int dr, int dc);

// Libera memoria asociada al puzzle
void puzzle_free(Puzzle *pz);

// Carga las texturas de los tiles desde una imagen
bool load_tiles_from_image(Gfx *gfx, Puzzle *pz, const char *path);

// Renderiza el puzzle en la ventana
void render(Gfx *gfx, Puzzle *pz, int win_size);

#endif // SDL_SLIDING_PUZZLE_H
