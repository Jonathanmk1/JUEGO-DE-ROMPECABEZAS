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

/* Para inyectar eventos por teclado si se desea */
void gr_debug_inject(GestureEvent e);

/* Helpers tiempo/hash para throttling de dibujo y telemetría */
static unsigned long long now_ms_u64(void){
#ifdef _WIN32
    return GetTickCount64();
#else
    return 0ULL;
#endif
}
static unsigned board_hash(const Puzzle* pu, GameState st){
    unsigned h = (unsigned)st ^ (unsigned)pu->moves_count ^ (unsigned)(pu->empty_r*31 + pu->empty_c*131);
    for (int r=0; r<pu->N; ++r)
        for (int c=0; c<pu->N; ++c)
            h = h*16777619u ^ (unsigned)pu->tiles[r][c];
    return h;
}
static void short_sleep_ms(int ms){
#ifdef _WIN32
    Sleep(ms);
#else
    (void)ms;
#endif
}

int main(int argc,char** argv){
    (void)argc;(void)argv;

    AppConfig cfg; cfg_load(&cfg,"config.json");

    if (ki_init()!=0){ fprintf(stderr,"[error] Kinect init\n"); return 1; }

    RenderCtx rc;
    RenderConfig rcfg;
    rcfg.width = 800;
    rcfg.height = 800;
    rcfg.hud_lang = "es";
    rcfg.show_hand_cursor = 1;
    rcfg.show_feedback_arrows = 1;
    re_init(&rc,&rcfg);

    Puzzle pu; pu_init(&pu,3);
    pu_shuffle(&pu,(unsigned)time(NULL),200);

    GestureParams gp = cfg.gp; gr_init(&gp);
    Session s; se_init(&s,3);

    /* Estado de dibujo/telemetría */
    static unsigned last_h = 0;
    static unsigned long long last_draw_ms = 0;
    static unsigned long long last_hand_ms = 0;

    re_draw(&rc,&pu,NULL, s.st==ST_READY, s.st==ST_PAUSE);
    printf("\nControles TEMP: WASD mueve, [p]=pausa/ready, [m]=mezcla, [q]=salir\n");

    for(;;){
        // 1) Leer Kinect (no bloqueante)
        HandSample right = {0}, left = {0};
        ki_read_hands(&right, &left);
        gr_push_sample(&right, &left);

        // 2) Teclado opcional sin bloquear
#ifdef _WIN32
        if (_kbhit()){
            int ch = _getch();
            if (ch == 0 || ch == 224) { _getch(); } // descarta flechas, F1...
            else {
                if (ch=='q'||ch=='Q') break;
                if (ch=='a'||ch=='A') gr_debug_inject(GEV_LEFT);
                if (ch=='d'||ch=='D') gr_debug_inject(GEV_RIGHT);
                if (ch=='w'||ch=='W') gr_debug_inject(GEV_UP);
                if (ch=='s'||ch=='S') gr_debug_inject(GEV_DOWN);
                if (ch=='p'||ch=='P') gr_debug_inject(GEV_PAUSE_TOGGLE);
                if (ch=='m'||ch=='M') gr_debug_inject(GEV_MIX);
            }
        }
#endif

        // 3) Procesar eventos de gesto
        GestureEvent ev;
        while (gr_poll_event(&ev)){
            se_handle_event(&s, ev, &pu);
            if (s.st == ST_MIX){
                pu_shuffle(&pu,(unsigned)time(NULL),(pu.N==3)?200:500);
                s.st = ST_PLAY;
            }
        }

        // 4) Victoria
        if (pu.is_solved && s.st==ST_PLAY){
            re_show_victory(&rc, pu.moves_count, 0.0);
        }

        // --- DEBUG: telemetría de mano cada 250ms (una línea) ---
        unsigned long long tnow = now_ms_u64();
        if (tnow - last_hand_ms >= 250) {
            printf("[hand] vis=%d  x=%.2f  y=%.2f  z=%.2f      \r",
                   right.visible, right.x, right.y, right.z);
            fflush(stdout);
            last_hand_ms = tnow;
        }

        // 5) Dibujar solo si cambió algo o cada 300ms
        unsigned h = board_hash(&pu, s.st);
        unsigned long long tdraw = now_ms_u64();
        if (h != last_h || (tdraw - last_draw_ms) >= 300) {
            re_draw(&rc,&pu,&right, s.st==ST_READY, s.st==ST_PAUSE);
            last_h = h;
            last_draw_ms = tdraw;
        }

        // 6) ~60 FPS
        short_sleep_ms(16);
    }

    re_shutdown(&rc);
    ki_shutdown();
    return 0;
}
