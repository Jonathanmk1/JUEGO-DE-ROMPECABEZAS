#include "gesture.h"
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
static double now_ms(void){ return (double)GetTickCount64(); }
#else
static double now_ms(void){ return 0.0; }
#endif

#define QUE_MAX 32
static GestureParams GP;
static GestureEvent QUE[QUE_MAX];
static int qh=0, qt=0;

static int q_push(GestureEvent e){
    int nxt = (qh+1)%QUE_MAX; if (nxt==qt) return 0; QUE[qh]=e; qh=nxt; return 1;
}
int gr_poll_event(GestureEvent* out_event){
    if (qt==qh) return 0; *out_event=QUE[qt]; qt=(qt+1)%QUE_MAX; return 1;
}

/* ===== Buffer de muestras (últimos ~1s) ===== */
#define MAX_SAMPLES 64
typedef struct { HandSample s[MAX_SAMPLES]; int n; } SampleBuf;
static SampleBuf RBUF, LBUF;
static double last_ev_ms = 0;   // cooldown

static void push_sample(SampleBuf* B, const HandSample* h){
    if (!h) return;
    if (B->n < MAX_SAMPLES) B->s[B->n++] = *h;
    else { memmove(&B->s[0], &B->s[1], sizeof(B->s[0])*(MAX_SAMPLES-1)); B->s[MAX_SAMPLES-1]=*h; }
}

static int oldest_index_in_window(const SampleBuf* B, double window_ms){
    if (B->n==0) return -1;
    double t_now = B->s[B->n-1].t_ms;
    int i = B->n-1;
    while (i>=0 && (t_now - B->s[i].t_ms) <= window_ms) i--;
    return (i < B->n-1) ? (i+1) : (B->n-1);
}

static void try_detect_swipe(const SampleBuf* B){
    if (B->n < 2) return;

    double t_now = B->s[B->n-1].t_ms;
    if (t_now - last_ev_ms < GP.cooldown_ms) return;

    int i0 = oldest_index_in_window(B, (double)GP.swipe_window_ms);
    if (i0 < 0) return;

    int i_first=-1, i_last=-1;
    for (int i=i0; i<B->n; ++i){ if (B->s[i].visible){ i_first=i; break; } }
    for (int i=B->n-1; i>=i0; --i){ if (B->s[i].visible){ i_last=i; break; } }
    if (i_first < 0 || i_last < 0 || i_last <= i_first) return;

    HandSample a = B->s[i_first];
    HandSample b = B->s[i_last];

    double dx = (double)b.x - (double)a.x;          // metros (X derecha +)
    double dy = (double)b.y - (double)a.y;          // metros (Y arriba +)
    double dt = (double)b.t_ms - (double)a.t_ms;    // ms
    if (dt <= 0) return;

    double dead_m   = GP.dead_zone_cm * 0.01;
    double swipe_m  = GP.swipe_threshold_cm * 0.01;
    double vmin_mps = GP.speed_min_cm_s * 0.01;

    double dist = sqrt(dx*dx + dy*dy);
    double speed = dist / (dt/1000.0);

    if (dist < dead_m) return;
    if (speed < vmin_mps) return;

    GestureEvent ev = GEV_NONE;
    double adx = fabs(dx), ady = fabs(dy);

    if (adx > ady && adx >= swipe_m){
        ev = (dx > 0) ? GEV_RIGHT : GEV_LEFT;
    } else if (ady >= swipe_m){
        ev = (dy > 0) ? GEV_UP : GEV_DOWN;
    }

    if (ev != GEV_NONE){
        printf("[gesture] swipe ev=%d dx=%.2f dy=%.2f speed=%.2f m/s\n",(int)ev,dx,dy,speed);
        q_push(ev);
        last_ev_ms = t_now;
    }
}

void gr_init(const GestureParams* p){
    if (p) GP=*p; else memset(&GP,0,sizeof(GP));
    qh=qt=0; RBUF.n=LBUF.n=0; last_ev_ms = 0;
}

void gr_push_sample(const HandSample* right, const HandSample* left){
    push_sample(&RBUF, right);
    push_sample(&LBUF, left);
    // Por ahora usamos mano derecha
    try_detect_swipe(&RBUF);
}

/* Debug helper (teclado opcional) */
void gr_debug_inject(GestureEvent e){ q_push(e); }
