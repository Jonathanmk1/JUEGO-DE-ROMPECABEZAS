// sdl_sliding_puzzle.c
// Rompecabezas deslizante 3x3 o 4x4 con una imagen.
// Controles: Flechas = mover fichas, R = mezclar, P = pausa, ESC = salir.
// Compila con SDL2 y SDL2_image.
// Autor: ChatGPT (reparación de lógica de imágenes)

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int N;                 // tamaño de la cuadrícula (3 o 4)
    int *board;            // tamaño N*N, valores 0..N*N-1; el último es el hueco (blank_idx)
    int blank_idx;         // índice del hueco en board
    int moves;
    int solved;            // 1 si está resuelto
} Puzzle;

typedef struct {
    SDL_Window   *win;
    SDL_Renderer *ren;
    SDL_Texture  *tex;     // textura de la imagen original
    int img_w, img_h;      // tamaño de la imagen cargada
    int win_size;          // ancho/alto de la ventana (cuadrada)
    int paused;
    char img_path[512];
} Gfx;

static void die(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

static int clampi(int v, int lo, int hi){ return v<lo?lo:(v>hi?hi:v); }

/* ---------- Utilidades de puzzle ---------- */

static void puzzle_init(Puzzle *p, int N) {
    p->N = N;
    p->board = (int*)malloc(sizeof(int) * N * N);
    for (int i=0;i<N*N;i++) p->board[i] = i;
    p->blank_idx = N*N - 1;
    p->moves = 0;
    p->solved = 1;
}

static void puzzle_free(Puzzle *p) {
    if (p->board) free(p->board);
    memset(p, 0, sizeof(*p));
}

static int count_inversions(const int *arr, int len) {
    int inv = 0;
    for (int i=0;i<len;i++) {
        if (arr[i] == len-1) continue; // ignorar hueco
        for (int j=i+1;j<len;j++) {
            if (arr[j] == len-1) continue;
            if (arr[i] > arr[j]) inv++;
        }
    }
    return inv;
}

static bool is_solvable(const Puzzle *p) {
    int N = p->N;
    int len = N*N;
    int inv = count_inversions(p->board, len);

    if ((N % 2) == 1) {
        // N impar: resoluble si #inversiones es par
        return (inv % 2) == 0;
    } else {
        // N par: depende de la fila del hueco contando desde abajo (1..N)
        int row_from_top = p->blank_idx / N;               // 0..N-1
        int row_from_bottom = N - row_from_top;            // 1..N
        // Si la fila del hueco desde abajo es par: #inv debe ser impar
        // Si la fila del hueco desde abajo es impar: #inv debe ser par
        if ((row_from_bottom % 2) == 0)
            return (inv % 2) == 1;
        else
            return (inv % 2) == 0;
    }
}

static void puzzle_shuffle(Puzzle *p, unsigned seed) {
    srand(seed ? seed : (unsigned)time(NULL));
    int len = p->N * p->N;

    // Fisher–Yates hasta conseguir una permutación resoluble
    int max_tries = 10000;
    for (int t=0;t<max_tries;t++) {
        // Permutación aleatoria 0..len-1
        for (int i=len-1;i>0;i--) {
            int j = rand() % (i+1);
            int tmp = p->board[i]; p->board[i] = p->board[j]; p->board[j] = tmp;
        }
        // localizar hueco
        for (int i=0;i<len;i++) if (p->board[i] == len-1) { p->blank_idx = i; break; }
        if (is_solvable(p)) break;
    }
    p->moves = 0;
    // considerar resuelto? normalmente no después de shuffle
    int ok = 1;
    for (int i=0;i<len;i++) if (p->board[i] != i) { ok = 0; break; }
    p->solved = ok;
}

static void puzzle_set_solved(Puzzle *p) {
    for (int i=0;i<p->N*p->N;i++) p->board[i] = i;
    p->blank_idx = p->N*p->N - 1;
    p->moves = 0;
    p->solved = 1;
}

static int puzzle_move(Puzzle *p, int dx, int dy) {
    if (p->solved) return 0;
    int N = p->N;
    int br = p->blank_idx / N;
    int bc = p->blank_idx % N;
    int tr = br - dy; // si presionas flecha arriba, el bloque debajo sube (y el hueco baja)
    int tc = bc - dx;

    if (tr < 0 || tr >= N || tc < 0 || tc >= N) return 0;

    int tile_idx = tr * N + tc;
    // intercambiar ficha y hueco
    int tmp = p->board[tile_idx];
    p->board[tile_idx] = p->board[p->blank_idx];
    p->board[p->blank_idx] = tmp;
    p->blank_idx = tile_idx;
    p->moves++;

    // comprobar resuelto
    int len = N*N;
    p->solved = 1;
    for (int i=0;i<len;i++) { if (p->board[i] != i) { p->solved = 0; break; } }
    return 1;
}

/* ---------- Render ---------- */

static void draw_grid(Gfx *g, const Puzzle *p) {
    int N = p->N;
    int s = g->win_size;
    int cell = s / N;

    SDL_SetRenderDrawBlendMode(g->ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->ren, 40, 40, 40, 255);
    for (int i=1;i<N;i++) {
        int x = i * cell;
        SDL_RenderDrawLine(g->ren, x, 0, x, s);
        SDL_RenderDrawLine(g->ren, 0, x, s, x);
    }
}

static void draw_tiles(Gfx *g, const Puzzle *p) {
    int N = p->N;
    int s = g->win_size;
    int cell = s / N;

    // Calcular región de la imagen para cada índice lógico (0..N*N-1), siendo el último el hueco
    for (int i=0;i<N*N;i++) {
        int tile = p->board[i];
        if (tile == N*N - 1) continue; // hueco: no dibujar

        int src_r = tile / N;
        int src_c = tile % N;

        SDL_Rect src = {
            .x = (g->img_w * src_c) / N,
            .y = (g->img_h * src_r) / N,
            .w = (g->img_w * (src_c+1)) / N - (g->img_w * src_c) / N,
            .h = (g->img_h * (src_r+1)) / N - (g->img_h * src_r) / N
        };

        int dst_r = i / N;
        int dst_c = i % N;

        SDL_Rect dst = {
            .x = dst_c * cell,
            .y = dst_r * cell,
            .w = cell,
            .h = cell
        };

        SDL_RenderCopy(g->ren, g->tex, &src, &dst);
    }
}

static void draw_hud(Gfx *g, const Puzzle *p) {
    // Rectángulo superior semi-transparente
    SDL_Rect bar = {0, 0, g->win_size, 32};
    SDL_SetRenderDrawBlendMode(g->ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->ren, 0, 0, 0, 160);
    SDL_RenderFillRect(g->ren, &bar);

    // No dibujamos texto (evitamos TTF). Dibujamos unas marcas simples:
    // barra de progreso proporcional al #de movimientos (capado a 200)
    int maxm = 200;
    int w = (p->moves > maxm ? maxm : p->moves) * g->win_size / maxm;
    SDL_SetRenderDrawColor(g->ren, 0, 180, 0, 200);
    SDL_Rect prog = {0, 0, w, 4};
    SDL_RenderFillRect(g->ren, &prog);

    // Borde si está resuelto o en pausa
    if (p->solved) {
        SDL_SetRenderDrawColor(g->ren, 0, 200, 0, 255);
    } else if (g->paused) {
        SDL_SetRenderDrawColor(g->ren, 200, 200, 0, 255);
    } else {
        SDL_SetRenderDrawColor(g->ren, 30, 30, 30, 255);
    }
    SDL_Rect border = {0, 0, g->win_size, g->win_size};
    SDL_RenderDrawRect(g->ren, &border);
}

static void render(Gfx *g, const Puzzle *p) {
    SDL_SetRenderDrawColor(g->ren, 15, 15, 18, 255);
    SDL_RenderClear(g->ren);
    draw_tiles(g, p);
    draw_grid(g, p);
    draw_hud(g, p);
    SDL_RenderPresent(g->ren);
}

/* ---------- App ---------- */

static void gfx_init(Gfx *g, const char *image_path, int win_size) {
    memset(g, 0, sizeof(*g));
    g->win_size = win_size;
    strncpy(g->img_path, image_path, sizeof(g->img_path)-1);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        die("No se pudo inicializar SDL.");
    }
    int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    int r = IMG_Init(flags);
    if ((r & flags) != flags) {
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        die("No se pudo inicializar SDL_image (PNG+JPG).");
    }

    g->win = SDL_CreateWindow("Rompecabezas SDL",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              win_size, win_size, SDL_WINDOW_SHOWN);
    if (!g->win) die("No se pudo crear la ventana.");

    g->ren = SDL_CreateRenderer(g->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->ren) die("No se pudo crear el renderer.");

    SDL_Surface *surf = IMG_Load(image_path);
    if (!surf) {
        fprintf(stderr, "IMG_Load('%s'): %s\n", image_path, IMG_GetError());
        die("No se pudo cargar la imagen.");
    }
    g->img_w = surf->w;
    g->img_h = surf->h;
    g->tex = SDL_CreateTextureFromSurface(g->ren, surf);
    SDL_FreeSurface(surf);
    if (!g->tex) die("No se pudo crear la textura a partir de la imagen.");
}

static void gfx_shutdown(Gfx *g) {
    if (g->tex) SDL_DestroyTexture(g->tex);
    if (g->ren) SDL_DestroyRenderer(g->ren);
    if (g->win) SDL_DestroyWindow(g->win);
    IMG_Quit();
    SDL_Quit();
}

static void usage(const char *exe){
    printf("Uso: %s --image <ruta> [--grid 3|4] [--size 800]\n", exe);
}

/* ---------- Programa principal ---------- */

int main(int argc, char **argv) {
    const char *img = NULL;
    int grid = 3;
    int size = 800;

    // Parseo simple de args
    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i], "--image")==0 && i+1<argc) { img = argv[++i]; continue; }
        if (strcmp(argv[i], "--grid")==0  && i+1<argc) { grid = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "--size")==0  && i+1<argc) { size = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "--help")==0) { usage(argv[0]); return 0; }
    }
    if (!img) { usage(argv[0]); return 1; }
    grid = clampi(grid, 3, 4);
    size = clampi(size, 400, 1400);

    Gfx gfx;
    gfx_init(&gfx, img, size);

    Puzzle pz;
    puzzle_init(&pz, grid);
    puzzle_shuffle(&pz, 0);

    int running = 1;
    Uint32 last_tick = SDL_GetTicks();

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = 0;
            else if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                    case SDLK_ESCAPE: running = 0; break;
                    case SDLK_r: puzzle_shuffle(&pz, (unsigned)time(NULL)); break;
                    case SDLK_p: gfx.paused = !gfx.paused; break;
                    case SDLK_UP:    if (!gfx.paused) puzzle_move(&pz, 0, +1); break;
                    case SDLK_DOWN:  if (!gfx.paused) puzzle_move(&pz, 0, -1); break;
                    case SDLK_LEFT:  if (!gfx.paused) puzzle_move(&pz, +1, 0); break;
                    case SDLK_RIGHT: if (!gfx.paused) puzzle_move(&pz, -1, 0); break;
                    default: break;
                }
            }
        }

        Uint32 now = SDL_GetTicks();
        if (now - last_tick >= 16) {
            render(&gfx, &pz);
            last_tick = now;
        }
        SDL_Delay(1);
    }

    puzzle_free(&pz);
    gfx_shutdown(&gfx);
    return 0;
}
