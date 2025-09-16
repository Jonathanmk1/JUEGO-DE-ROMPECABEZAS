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

/* ====== MODO (sin spam) ====== */
#define DEBUG_BYPASS_ARMING 0   /* 0: exige quietud breve antes del siguiente swipe */
/* ============================ */

/* Parámetros internos */
#define QUE_MAX 32
#define MAX_SAMPLES 64

/* Arming (mano quieta) — tolerancias algo permisivas */
#define ARM_DWELL_MS        350   /* tiempo mínimo quieto (ms) */
#define STILL_SPEED_MAX_MPS 0.35  /* velocidad máx. para considerar quieto (m/s) */
#define STILL_DRIFT_CM      8.0   /* deriva permitida durante dwell (cm) */

/* Manos juntas (pausa/salir) */
#define HANDS_NEAR_CM       12.0
#define HANDS_Z_DIFF_CM     15.0
#define HANDS_STILL_MPS     0.15
#define PAUSE_HOLD_MS        600
#define QUIT_HOLD_MS        1800

/* Buffers/estado */
static GestureParams GP;
static GestureEvent QUE[QUE_MAX]; static int qh=0, qt=0;
typedef struct { HandSample s[MAX_SAMPLES]; int n; } SampleBuf;
static SampleBuf RBUF, LBUF;

static double last_ev_ms = 0;
static int    armed = 0;  static double arm_start_ms = 0; static HandSample arm_ref = {0};
static double hands_start_ms = 0; static int hands_active = 0;

static int q_push(GestureEvent e){ int nx=(qh+1)%QUE_MAX; if(nx==qt) return 0; QUE[qh]=e; qh=nx; return 1; }
int gr_poll_event(GestureEvent* out){ if(qt==qh) return 0; *out=QUE[qt]; qt=(qt+1)%QUE_MAX; return 1; }

static void push_sample(SampleBuf* B, const HandSample* h){
    if(!h) return;
    if(B->n<MAX_SAMPLES) B->s[B->n++]=*h;
    else { memmove(&B->s[0],&B->s[1],sizeof(B->s[0])*(MAX_SAMPLES-1)); B->s[MAX_SAMPLES-1]=*h; }
}

static int oldest_index_in_window(const SampleBuf* B, double window_ms){
    if (B->n==0) return -1;
    double t_now=B->s[B->n-1].t_ms; int i=B->n-1;
    while(i>=0 && (t_now - B->s[i].t_ms) <= window_ms) i--;
    return (i < B->n-1) ? (i+1) : (B->n-1);
}

/* ===== Arming (mano quieta) ===== */
static void update_arming(const SampleBuf* B){
    if(B->n<2) return;
    const HandSample *a=&B->s[(B->n>=6)?B->n-6:0], *b=&B->s[B->n-1];

    double dt=(b->t_ms-a->t_ms)/1000.0; if(dt<=0) return;
    double dx=b->x-a->x, dy=b->y-a->y, dz=b->z-a->z;
    double speed=sqrt(dx*dx+dy*dy+dz*dz)/dt;

    double t_now=b->t_ms;

    if(!armed){
        if(arm_start_ms==0 || !arm_ref.visible){ arm_start_ms=t_now; arm_ref=*b; }

        double ddx=b->x-arm_ref.x, ddy=b->y-arm_ref.y, ddz=b->z-arm_ref.z;
        double drift=sqrt(ddx*ddx+ddy*ddy+ddz*ddz);

        int visible_ok = b->visible;
        int speed_ok   = (speed <= STILL_SPEED_MAX_MPS);
        int drift_ok   = (drift <= STILL_DRIFT_CM*0.01);

        if(!visible_ok || !speed_ok || !drift_ok){
            arm_start_ms = t_now; arm_ref = *b;
        } else if ((t_now - arm_start_ms) >= ARM_DWELL_MS){
            armed = 1;
            /* printf("[armed]\n");  // silencioso */
        }
    } else {
        if(speed > STILL_SPEED_MAX_MPS*1.5){
            armed = 0; arm_start_ms = t_now; arm_ref = *b;
            /* printf("[disarmed]\n"); */
        }
    }
}

/* ===== Manos juntas (pausa/salir) ===== */
static void detect_hands_together(const SampleBuf* R, const SampleBuf* L){
    if(R->n==0 || L->n==0) return;

    const HandSample *br=&R->s[R->n-1], *bl=&L->s[L->n-1];
    if(!br->visible || !bl->visible){ hands_active=0; hands_start_ms=0; return; }

    double dx=br->x-bl->x, dy=br->y-bl->y, dz=br->z-bl->z;
    double dist = sqrt(dx*dx + dy*dy + dz*dz);

    int near_ok = (dist <= HANDS_NEAR_CM*0.01);
    int z_ok    = (fabs(dz) <= HANDS_Z_DIFF_CM*0.01);

    /* estabilidad simple */
    const int k=3;
    if (R->n<=k || L->n<=k){ hands_active=0; hands_start_ms=0; return; }
    const HandSample *rr=&R->s[R->n-1-k], *ll=&L->s[L->n-1-k];
    double dtR=(br->t_ms-rr->t_ms)/1000.0, dtL=(bl->t_ms-ll->t_ms)/1000.0;
    if (dtR<=0 || dtL<=0){ hands_active=0; hands_start_ms=0; return; }
    double vR = sqrt((br->x-rr->x)*(br->x-rr->x)+(br->y-rr->y)*(br->y-rr->y)+(br->z-rr->z)*(br->z-rr->z))/dtR;
    double vL = sqrt((bl->x-ll->x)*(bl->x-ll->x)+(bl->y-ll->y)*(bl->y-ll->y)+(bl->z-ll->z)*(bl->z-ll->z))/dtL;
    int still_ok = (vR<=HANDS_STILL_MPS && vL<=HANDS_STILL_MPS);

    double t_now = br->t_ms;
    if (near_ok && z_ok && still_ok){
        if(!hands_active){ hands_active=1; hands_start_ms=t_now; }
        else {
            double held = t_now - hands_start_ms;
            if (held >= QUIT_HOLD_MS){
                q_push(GEV_QUIT); hands_active=0; hands_start_ms=0;
            } else if (held >= PAUSE_HOLD_MS){
                q_push(GEV_PAUSE_TOGGLE); hands_active=0; hands_start_ms=0;
            }
        }
    } else {
        hands_active=0; hands_start_ms=0;
    }
}

/* ===== Swipes ===== */
static void try_detect_swipe(const SampleBuf* B){
    if(B->n<2) return;
    double t_now=B->s[B->n-1].t_ms;

#if !DEBUG_BYPASS_ARMING
    if(!armed) return;    /* <- clave: exige estar quieto (evita el “regreso” accidental) */
#endif
    if(t_now - last_ev_ms < GP.cooldown_ms) return;

    int i0 = oldest_index_in_window(B,(double)GP.swipe_window_ms);
    if(i0<0) return;

    int i_first=-1,i_last=-1;
    for(int i=i0;i<B->n;i++) if(B->s[i].visible){ i_first=i; break; }
    for(int i=B->n-1;i>=i0;i--) if(B->s[i].visible){ i_last=i; break; }
    if(i_first<0 || i_last<=i_first) return;

    HandSample a=B->s[i_first], b=B->s[i_last];
    double dx=b.x-a.x, dy=b.y-a.y, dz=b.z-a.z;
    double dt=(b.t_ms-a.t_ms)/1000.0; if(dt<=0) return;

    double dead_m=GP.dead_zone_cm*0.01, swipe_m=GP.swipe_threshold_cm*0.01;
    double vmin=GP.speed_min_cm_s*0.01;

    double dist = sqrt(dx*dx+dy*dy);
    double speed= dist/dt;

    if(dist<dead_m) return;
    if(speed<vmin) return;
    if(fabs(dz)>0.20) return;

    double adx=fabs(dx), ady=fabs(dy);
    double dir_ratio = (adx>ady)? adx/(ady+1e-6) : ady/(adx+1e-6);
    if(dir_ratio<1.25) return;

    GestureEvent ev=GEV_NONE;
    if(adx>ady && adx>=swipe_m)   ev = (dx>0)? GEV_RIGHT:GEV_LEFT;
    else if(ady>=swipe_m)        ev = (dy>0)? GEV_UP   :GEV_DOWN;

    if(ev!=GEV_NONE){
        /* printf("[gesture] SWIPE ev=%d dx=%.2f dy=%.2f speed=%.2f m/s\n",(int)ev,dx,dy,speed); */
        q_push(ev);
        last_ev_ms=t_now;

#if !DEBUG_BYPASS_ARMING
        /* Tras un swipe, desarma: el back-swing ya NO cuenta. */
        armed = 0;
        arm_start_ms = t_now;
        arm_ref = B->s[B->n-1];
#endif
    }
}

void gr_init(const GestureParams* p){
    if(p) GP=*p; else memset(&GP,0,sizeof(GP));
    qh=qt=0; RBUF.n=LBUF.n=0;
    last_ev_ms=0; armed=0; arm_start_ms=0; memset(&arm_ref,0,sizeof(arm_ref));
    hands_active=0; hands_start_ms=0;
}

void gr_push_sample(const HandSample* right, const HandSample* left){
    push_sample(&RBUF,right); push_sample(&LBUF,left);
    detect_hands_together(&RBUF,&LBUF);
    update_arming(&RBUF);
    try_detect_swipe(&RBUF);
}

void gr_debug_inject(GestureEvent e){ q_push(e); }
