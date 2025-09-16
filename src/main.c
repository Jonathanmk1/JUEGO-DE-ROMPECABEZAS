#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#endif

#include "puzzle.h"
#include "gesture.h"
#include "kinect_input.h"
#include "renderer.h"
#include "session.h"
#include "config.h"

/* ======== MODO PASO A PASO ======== */
#define MOVE_PAUSE_MS  2800   /* tiempo para pensar tras cada jugada */
#define IDLE_SLEEP_MS    20   /* respiro de la espera */
/* ================================= */

static void short_sleep_ms(int ms){
#ifdef _WIN32
    Sleep(ms);
#else
    (void)ms;
#endif
}

static const char* gesture_name(GestureEvent ev){
    switch(ev){
        case GEV_LEFT:  return "IZQUIERDA";
        case GEV_RIGHT: return "DERECHA";
        case GEV_UP:    return "ARRIBA";
        case GEV_DOWN:  return "ABAJO";
        case GEV_MIX:   return "MEZCLA";
        case GEV_PAUSE_TOGGLE: return "PAUSA/READY";
        case GEV_QUIT:  return "SALIR";
        default:        return "NINGUNO";
    }
}

static int poll_keyboard_and_inject(void){
#ifdef _WIN32
    if (_kbhit()){
        int ch = _getch();
        if (ch == 0 || ch == 224) { _getch(); return 0; } // ignora teclas especiales
        switch(ch){
            case 'q': case 'Q': return -1;                           // salir inmediato
            case 'a': case 'A': gr_debug_inject(GEV_LEFT);  return 1;
            case 'd': case 'D': gr_debug_inject(GEV_RIGHT); return 1;
            case 'w': case 'W': gr_debug_inject(GEV_UP);    return 1;
            case 's': case 'S': gr_debug_inject(GEV_DOWN);  return 1;
            case 'p': case 'P': gr_debug_inject(GEV_PAUSE_TOGGLE); return 1;
            case 'm': case 'M': gr_debug_inject(GEV_MIX);   return 1;
            default: return 0;
        }
    }
#endif
    return 0;
}

int main(int argc,char** argv){
    (void)argc;(void)argv;

    AppConfig cfg; cfg_load(&cfg,"config.json");
    if (ki_init()!=0){ fprintf(stderr,"[error] Kinect init\n"); return 1; }

    RenderCtx rc; RenderConfig rcfg;
    rcfg.width = 800; rcfg.height = 800;
    rcfg.hud_lang = "es";
    rcfg.show_hand_cursor = 1;
    rcfg.show_feedback_arrows = 1;
    re_init(&rc,&rcfg);

    Puzzle pu; pu_init(&pu,3);
    pu_shuffle(&pu,(unsigned)time(NULL),200);

    GestureParams gp = cfg.gp; gr_init(&gp);

    Session s; se_init(&s,3);

    re_draw(&rc,&pu,NULL, s.st==ST_READY, s.st==ST_PAUSE);
    printf("\nControles: WASD=mov debug | p=pause/ready | m=mezcla | q=salir\n");
    printf("Modo paso-a-paso: pausa automática tras cada gesto para analizar.\n");
    printf("Consejo: pulsa 'p' para salir de [READY] a PLAY.\n\n");

    while (1){
        /* ===== Espera un gesto/tecla (consola silenciosa) ===== */
        GestureEvent ev = GEV_NONE;
        printf("[espera] Haz un gesto (swipe). También puedes usar WASD/p/m.\n");

        while (ev == GEV_NONE){
            HandSample right={0}, left={0};
            ki_read_hands(&right,&left);
            gr_push_sample(&right,&left);

            int kb = poll_keyboard_and_inject();
            if (kb < 0) { re_shutdown(&rc); ki_shutdown(); return 0; }

            if (gr_poll_event(&ev)){
                break;
            }
            short_sleep_ms(IDLE_SLEEP_MS);
        }

        /* ===== Procesa el gesto ===== */
        if (ev == GEV_QUIT){
            re_draw(&rc,&pu,NULL, s.st==ST_READY, s.st==ST_PAUSE);
            printf("\n[gesture] %s\n", gesture_name(ev));
            printf("[info] Saliendo por gesto de SALIR.\n");
            break;
        }

        se_handle_event(&s, ev, &pu);
        if (s.st == ST_MIX){
            pu_shuffle(&pu,(unsigned)time(NULL),(pu.N==3)?200:500);
            s.st = ST_PLAY;
        }

        re_draw(&rc,&pu,NULL, s.st==ST_READY, s.st==ST_PAUSE);
        printf("\n[gesture] %s\n", gesture_name(ev));
        printf("[mov] count=%d  empty=(%d,%d)\n", pu.moves_count, pu.empty_r, pu.empty_c);
        if (s.st == ST_READY) printf("[estado] READY: pulsa 'p' para entrar a PLAY.\n");
        if (s.st == ST_PAUSE) printf("[estado] PAUSE: pulsa 'p' para reanudar.\n");

        short_sleep_ms(MOVE_PAUSE_MS);
    }

    re_shutdown(&rc);
    ki_shutdown();
    return 0;
}
