#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    int initted = IMG_Init(flags);
    if ((initted & flags) != flags) {
        printf("IMG_Init: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    printf("✅ SDL_image soporta PNG y JPG correctamente.\n");

    IMG_Quit();
    SDL_Quit();
    return 0;
}
