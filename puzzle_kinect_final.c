// *********************************************************************
// puzzle_kinect_final.c — Menú principal para "Puzzle Kinect 3D"
// Ejecuta el rompecabezas en la MISMA ventana/renderer (sin abrir otra).
// Requiere game_bridge.h/cpp y refactor mínimo en src/main_sdl_kinect.cpp.
// *********************************************************************

#define _CRT_SECURE_NO_WARNINGS
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <SDL.h>
#include <SDL_syswm.h>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <commdlg.h>
  #include <shlwapi.h>
  #pragma comment(lib, "Comdlg32.lib")
  #pragma comment(lib, "Shlwapi.lib")
#endif

#include "game_bridge.h"  // <— NUEVO: puente hacia el juego

// ------------------- Layout base -------------------
static const int BASE_W = 1366;
static const int BASE_H = 768;
static const int TARGET_FPS = 60;

static float g_ui_scale = 1.0f;
static int   g_winW = BASE_W, g_winH = BASE_H;

static const SDL_Color COL_BG      = { 12, 20, 34, 255 };
static const SDL_Color COL_TITLE   = { 180, 210, 240, 255 };
static const SDL_Color COL_SUB     = { 120, 150, 190, 255 };
static const SDL_Color COL_LABEL   = { 210, 220, 230, 255 };
static const SDL_Color COL_HOVER   = { 255, 255,   0, 255 };

static const SDL_Color COL_V_CYAN  = {  20, 220, 230, 255 };
static const SDL_Color COL_V_GREEN = {  20, 210,  80, 255 };
static const SDL_Color COL_V_PINK  = { 220,  50, 220, 255 };
static const SDL_Color COL_V_ORANGE= { 230, 140,  20, 255 };

// ------------------- Micro-font 5x7 -------------------
typedef unsigned char u8;
static u8 FONT5x7[96][7];

#define SETGLYPH(CH,A,B,C,D,E,F,G) do { \
  u8 _r[7] = { (A),(B),(C),(D),(E),(F),(G) }; \
  memcpy(FONT5x7[(CH)-32], _r, 7); \
} while(0)

static void font_build(void){
  memset(FONT5x7, 0, sizeof(FONT5x7));
  SETGLYPH(' ',0,0,0,0,0,0,0);
  SETGLYPH('0',0x1E,0x11,0x13,0x15,0x19,0x11,0x1E);
  SETGLYPH('1',0x04,0x0C,0x14,0x04,0x04,0x04,0x1F);
  SETGLYPH('2',0x1E,0x01,0x01,0x1E,0x10,0x10,0x1F);
  SETGLYPH('3',0x1E,0x01,0x01,0x0E,0x01,0x01,0x1E);
  SETGLYPH('4',0x02,0x06,0x0A,0x12,0x1F,0x02,0x02);
  SETGLYPH('5',0x1F,0x10,0x10,0x1E,0x01,0x01,0x1E);
  SETGLYPH('6',0x0E,0x10,0x10,0x1E,0x11,0x11,0x1E);
  SETGLYPH('7',0x1F,0x01,0x02,0x04,0x08,0x08,0x08);
  SETGLYPH('8',0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E);
  SETGLYPH('9',0x1E,0x11,0x11,0x1E,0x01,0x01,0x0E);
  SETGLYPH('A',0x0E,0x11,0x11,0x1F,0x11,0x11,0x11);
  SETGLYPH('B',0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E);
  SETGLYPH('C',0x0E,0x11,0x10,0x10,0x10,0x11,0x0E);
  SETGLYPH('D',0x1C,0x12,0x11,0x11,0x11,0x12,0x1C);
  SETGLYPH('E',0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F);
  SETGLYPH('F',0x1F,0x10,0x10,0x1E,0x10,0x10,0x10);
  SETGLYPH('G',0x0E,0x11,0x10,0x17,0x11,0x11,0x0E);
  SETGLYPH('H',0x11,0x11,0x11,0x1F,0x11,0x11,0x11);
  SETGLYPH('I',0x1F,0x04,0x04,0x04,0x04,0x04,0x1F);
  SETGLYPH('J',0x1F,0x01,0x01,0x01,0x11,0x11,0x0E);
  SETGLYPH('K',0x11,0x12,0x14,0x18,0x14,0x12,0x11);
  SETGLYPH('L',0x10,0x10,0x10,0x10,0x10,0x10,0x1F);
  SETGLYPH('M',0x11,0x1B,0x15,0x11,0x11,0x11,0x11);
  SETGLYPH('N',0x11,0x19,0x15,0x13,0x11,0x11,0x11);
  SETGLYPH('O',0x0E,0x11,0x11,0x11,0x11,0x11,0x0E);
  SETGLYPH('P',0x1E,0x11,0x11,0x1E,0x10,0x10,0x10);
  SETGLYPH('Q',0x0E,0x11,0x11,0x11,0x15,0x12,0x0D);
  SETGLYPH('R',0x1E,0x11,0x11,0x1E,0x14,0x12,0x11);
  SETGLYPH('S',0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E);
  SETGLYPH('T',0x1F,0x04,0x04,0x04,0x04,0x04,0x04);
  SETGLYPH('U',0x11,0x11,0x11,0x11,0x11,0x11,0x0E);
  SETGLYPH('V',0x11,0x11,0x11,0x11,0x11,0x0A,0x04);
  SETGLYPH('W',0x11,0x11,0x11,0x11,0x15,0x1B,0x11);
  SETGLYPH('X',0x11,0x11,0x0A,0x04,0x0A,0x11,0x11);
  SETGLYPH('Y',0x11,0x11,0x0A,0x04,0x04,0x04,0x04);
  SETGLYPH('Z',0x1F,0x01,0x02,0x04,0x08,0x10,0x1F);
  SETGLYPH(':',0x00,0x04,0x00,0x00,0x04,0x00,0x00);
  SETGLYPH('.',0x00,0x00,0x00,0x00,0x00,0x0C,0x0C);
}

static void draw_text(SDL_Renderer* R, int x, int y, const char* s, int scale, SDL_Color c){
  if(scale < 1) scale = 1;
  SDL_SetRenderDrawColor(R, c.r, c.g, c.b, c.a);
  int cx=x;
  for(const char* p=s; *p; ++p){
    unsigned char ch = (unsigned char)*p;
    if(ch<32 || ch>127){ cx += 6*scale; continue; }
    const u8* rows = FONT5x7[ch-32];
    for(int ry=0; ry<7; ++ry){
      u8 bits = rows[ry];
      for(int rx=0; rx<5; ++rx){
        if(bits & (1<<(4-rx))){
          SDL_Rect r = { cx + rx*scale, y + ry*scale, scale, scale };
          SDL_RenderFillRect(R, &r);
        }
      }
    }
    cx += 6*scale;
  }
}

// ------------------- Helpers / Backend -------------------
static char g_ruta_imagen[MAX_PATH] = "assets\\LogoUAEMex.png";
static const int G_GRIDS[3] = {2,3,4};
static int g_nivel_idx = 0; // 0..2

static int seleccionar_imagen(void){
#ifdef _WIN32
  OPENFILENAMEA ofn; ZeroMemory(&ofn, sizeof(ofn));
  char file[MAX_PATH] = "";
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFilter = "Imagenes\0*.png;*.jpg;*.jpeg\0Todos\0*.*\0";
  ofn.lpstrFile   = file;
  ofn.nMaxFile    = MAX_PATH;
  ofn.Flags       = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
  if (GetOpenFileNameA(&ofn)) { strcpy(g_ruta_imagen, file); return 1; }
#endif
  return 0;
}

// ------------------- Fondo: estrellas -------------------
typedef struct { float x,y,vx,vy,size; int r,g,b; } Star;
#define MAX_STARS 180
static Star g_stars[MAX_STARS];

static void stars_init(int w,int h){
  int i; srand((unsigned)time(NULL));
  for(i=0;i<MAX_STARS;i++){
    g_stars[i].x = (float)(rand()%w);
    g_stars[i].y = (float)(rand()%h);
    g_stars[i].vx = (rand()%100-50)/500.0f;
    g_stars[i].vy = (rand()%100-50)/500.0f;
    g_stars[i].r = g_stars[i].g = g_stars[i].b = 180 + rand()%60;
    g_stars[i].size = (rand()%2)? 1.0f: 2.0f;
  }
}
static void stars_update(int w,int h){
  for(int i=0;i<MAX_STARS;i++){
    g_stars[i].x += g_stars[i].vx;
    g_stars[i].y += g_stars[i].vy;
    if(g_stars[i].x<0) g_stars[i].x+=w; if(g_stars[i].x>=w) g_stars[i].x-=w;
    if(g_stars[i].y<0) g_stars[i].y+=h; if(g_stars[i].y>=h) g_stars[i].y-=h;
  }
}
static void stars_draw(SDL_Renderer* R){
  for(int i=0;i<MAX_STARS;i++){
    SDL_SetRenderDrawColor(R, g_stars[i].r, g_stars[i].g, g_stars[i].b, 210);
    int s = (int)(g_stars[i].size * fmaxf(1.0f, g_ui_scale*0.8f));
    SDL_Rect d = { (int)g_stars[i].x, (int)g_stars[i].y, s, s };
    SDL_RenderFillRect(R, &d);
  }
}

// ------------------- Vórtices -------------------
typedef struct {
  float cx, cy;
  float radius;
  int   arms;
  SDL_Color color;
  const char* label;
  int hover;
} Vortex;

static void draw_vortex(SDL_Renderer* R, const Vortex* v, float t){
  float rad = v->radius * g_ui_scale;
  int   ring = (int)roundf(26.0f * g_ui_scale);
  SDL_SetRenderDrawColor(R, 40, 50, 70, 160);
  for(int r=(int)rad+ (int)roundf(22*g_ui_scale); r>(int)rad+ (int)roundf(18*g_ui_scale); --r){
    for(float a=0; a<6.2831f; a+=0.05f){
      int x = (int)(v->cx + cosf(a)*r);
      int y = (int)(v->cy + sinf(a)*r);
      if(x>=0 && x<g_winW && y>=0 && y<g_winH) SDL_RenderDrawPoint(R, x, y);
    }
  }
  SDL_SetRenderDrawColor(R, v->color.r, v->color.g, v->color.b, 255);
  for(int i=0;i<v->arms;i++){
    float ang = (6.2831f * i / v->arms) + sinf(t*0.9f + i*0.15f)*0.15f;
    float r1 = rad * (0.35f + 0.15f*sinf(t*1.7f + i));
    float r2 = rad * (0.95f + 0.25f*sinf(t*1.3f + i*0.7f));
    int x1=(int)(v->cx + cosf(ang)*r1), y1=(int)(v->cy + sinf(ang)*r1);
    int x2=(int)(v->cx + cosf(ang)*r2), y2=(int)(v->cy + sinf(ang)*r2);
    SDL_RenderDrawLine(R, x1,y1,x2,y2);
  }
  SDL_SetRenderDrawColor(R, 20, 20, 28, 255);
  for(int r=(int)roundf(10*g_ui_scale); r>(int)roundf(6*g_ui_scale); --r){
    for(float a=0; a<6.2831f; a+=0.03f){
      int x = (int)(v->cx + cosf(a)*r);
      int y = (int)(v->cy + sinf(a)*r);
      if(x>=0 && x<g_winW && y>=0 && y<g_winH) SDL_RenderDrawPoint(R, x, y);
    }
  }
  if(v->hover){
    SDL_SetRenderDrawColor(R, COL_HOVER.r, COL_HOVER.g, COL_HOVER.b, 255);
    for(float a=0;a<6.2831f;a+=0.01f){
      int x = (int)(v->cx + cosf(a)*(rad+ring));
      int y = (int)(v->cy + sinf(a)*(rad+ring));
      if(x>=0 && x<g_winW && y>=0 && y<g_winH) SDL_RenderDrawPoint(R, x, y);
    }
  }
  int ts = (int)fmaxf(1.0f, roundf(2*g_ui_scale));
  int label_y = (int)(v->cy + rad + (int)roundf(28*g_ui_scale));
  int label_x = (int)(v->cx - (int)strlen(v->label)*3*ts);
  if(label_y > g_winH-10) label_y = g_winH-10;
  if(label_x < 10) label_x = 10;
  draw_text(R, label_x, label_y, v->label, ts, COL_LABEL);
}

static float dist2(float x1,float y1,float x2,float y2){
  float dx=x1-x2, dy=y1-y2; return dx*dx+dy*dy;
}

// ------------------- Escala/DPI -------------------
static void update_ui_scale(SDL_Window* win, SDL_Renderer* R){
  int rw, rh; SDL_GetRendererOutputSize(R, &rw, &rh);
  g_winW = rw; g_winH = rh;

  float sx = (float)rw / (float)BASE_W;
  float sy = (float)rh / (float)BASE_H;
  g_ui_scale = fminf(sx, sy);
  if (g_ui_scale < 0.5f) g_ui_scale = 0.5f;
  if (g_ui_scale > 2.0f) g_ui_scale = 2.0f;
}

// ------------------- Main -------------------
int main(int argc, char** argv){
  (void)argc; (void)argv;
  font_build();

  SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

  if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0){
    fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError()); return 1;
  }

  SDL_Rect usable; 
  if (SDL_GetDisplayUsableBounds(0, &usable) != 0) { usable.x=0; usable.y=0; usable.w=BASE_W; usable.h=BASE_H; }
  int startW = (BASE_W > usable.w) ? usable.w : BASE_W;
  int startH = (BASE_H > usable.h) ? usable.h : BASE_H;

  SDL_Window* win = SDL_CreateWindow(
      "PUZZLE KINECT 3D — Menu",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      startW, startH,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
  );
  if(!win){ fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); return 1; }

  SDL_Renderer* R = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
  if(!R){ fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError()); return 1; }

  update_ui_scale(win, R);
  stars_init(g_winW, g_winH);

  Vortex iniciar = { g_winW*0.33f, g_winH*0.35f, 90.f, 48, COL_V_CYAN,  "INICIAR JUEGO", 0 };
  Vortex cargar  = { g_winW*0.66f, g_winH*0.35f, 90.f, 48, COL_V_GREEN, "CARGAR IMAGEN", 0 };
  Vortex nivel   = { g_winW*0.33f, g_winH*0.68f, 90.f, 48, COL_V_PINK,  "NIVEL",         0 };
  Vortex salir   = { g_winW*0.66f, g_winH*0.68f, 90.f, 48, COL_V_ORANGE,"SALIR",         0 };

  int running = 1;
  int mx=0, my=0;
  int mdown=0;

  while(running){
    SDL_Event e;
    while(SDL_PollEvent(&e)){
      if(e.type==SDL_QUIT) running=0;
      else if(e.type==SDL_WINDOWEVENT){
        if(e.window.event==SDL_WINDOWEVENT_SIZE_CHANGED ||
           e.window.event==SDL_WINDOWEVENT_RESIZED ||
           e.window.event==SDL_WINDOWEVENT_MAXIMIZED ||
           e.window.event==SDL_WINDOWEVENT_RESTORED){
          update_ui_scale(win, R);
          iniciar.cx = g_winW*0.33f; iniciar.cy = g_winH*0.35f;
          cargar .cx = g_winW*0.66f; cargar .cy = g_winH*0.35f;
          nivel  .cx = g_winW*0.33f; nivel  .cy = g_winH*0.68f;
          salir  .cx = g_winW*0.66f; salir  .cy = g_winH*0.68f;
        }
      }
      else if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_F11){
        Uint32 flags = SDL_GetWindowFlags(win);
        if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) SDL_SetWindowFullscreen(win, 0);
        else SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
        update_ui_scale(win, R);
        iniciar.cx = g_winW*0.33f; iniciar.cy = g_winH*0.35f;
        cargar .cx = g_winW*0.66f; cargar .cy = g_winH*0.35f;
        nivel  .cx = g_winW*0.33f; nivel  .cy = g_winH*0.68f;
        salir  .cx = g_winW*0.66f; salir  .cy = g_winH*0.68f;
      }
      else if(e.type==SDL_MOUSEMOTION){ mx=e.motion.x; my=e.motion.y; }
      else if(e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT){ mdown=1; }
      else if(e.type==SDL_MOUSEBUTTONUP   && e.button.button==SDL_BUTTON_LEFT){
        if(mdown){
          float rr = (90.f*g_ui_scale + 24.f*g_ui_scale); float rr2 = rr*rr;
          if( dist2(mx,my,iniciar.cx,iniciar.cy) <= rr2 ){
            int grid = G_GRIDS[g_nivel_idx];
            // *** AQUÍ: ejecutar el juego dentro de la MISMA ventana/renderer ***
            run_puzzle_game_c(g_ruta_imagen, grid, 800, win, R);
            // Al volver, seguimos en el menú
          } else if( dist2(mx,my,cargar.cx,cargar.cy) <= rr2 ){
            seleccionar_imagen();
          } else if( dist2(mx,my,nivel.cx,nivel.cy) <= rr2 ){
            g_nivel_idx = (g_nivel_idx + 1) % 3;
          } else if( dist2(mx,my,salir.cx,salir.cy) <= rr2 ){
            running=0;
          }
        }
        mdown=0;
      }
    }

    float rr = (90.f*g_ui_scale + 24.f*g_ui_scale); float rr2 = rr*rr;
    iniciar.hover = dist2(mx,my,iniciar.cx,iniciar.cy) <= rr2;
    cargar .hover = dist2(mx,my,cargar .cx,cargar .cy) <= rr2;
    nivel  .hover = dist2(mx,my,nivel  .cx,nivel  .cy) <= rr2;
    salir  .hover = dist2(mx,my,salir  .cx,salir  .cy) <= rr2;

    stars_update(g_winW, g_winH);

    SDL_SetRenderDrawColor(R, COL_BG.r, COL_BG.g, COL_BG.b, 255);
    SDL_RenderClear(R);

    stars_draw(R);

    int tsTitle = (int)fmaxf(1.0f, roundf(3*g_ui_scale));
    int tsSub   = (int)fmaxf(1.0f, roundf(2*g_ui_scale));
    draw_text(R, g_winW/2 - 6*18*tsTitle/3, (int)roundf(30*g_ui_scale), "PUZZLE KINECT 3D", tsTitle, COL_TITLE);
    draw_text(R, g_winW/2 - 6*16*tsSub/2,   (int)roundf(80*g_ui_scale), "EXPLORA  CREA  RESUELVE", tsSub, COL_SUB);

    float t = SDL_GetTicks()/1000.0f;
    draw_vortex(R, &iniciar, t);
    draw_vortex(R, &cargar,  t);
    draw_vortex(R, &nivel,   t);
    draw_vortex(R, &salir,   t);

    const char* etiquetas[3] = {"2x2","3x3","4x4"};
    char info[512];
    int g = G_GRIDS[g_nivel_idx];
    snprintf(info, sizeof(info), "TAMANO: %s   GRID: %dx%d   IMG: %s",
             etiquetas[g_nivel_idx], g, g, g_ruta_imagen);
    int ts = (int)fmaxf(1.0f, roundf(2*g_ui_scale));
    int y  = g_winH - (int)roundf(28*g_ui_scale); if(y < 10) y = 10;
    draw_text(R, 20, y, info, ts, COL_SUB);

    int cs = (int)fmaxf(4.0f, roundf(6*g_ui_scale));
    SDL_SetRenderDrawColor(R, 255,255,0,255);
    SDL_Rect c = { mx - cs/2, my - cs/2, cs, cs }; SDL_RenderFillRect(R, &c);

    SDL_RenderPresent(R);
    SDL_Delay(1000 / TARGET_FPS);
  }

  SDL_DestroyRenderer(R);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
