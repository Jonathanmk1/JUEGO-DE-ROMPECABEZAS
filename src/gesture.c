#include "gesture.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
static double now_ms(void){ return (double)GetTickCount64(); }
#else
static double now_ms(void){ return 0.0; }
#endif

/* ==========================
   PARÁMETROS Y CONSTANTES
   ========================== */

/* Swipes (mano derecha) – tolerantes pero razonables */
#define ARM_DWELL_MS          300       /* quieto para “armar” */
#define STILL_SPEED_MAX_MPS   0.70      /* tolerancia de micro-movimiento */
#define STILL_DRIFT_CM        12.0

/* Ventanas swipe (se afinan con config.c también) */
#define DIR_RATIO_MIN         1.08      /* permite diagonales leves */
#define DZ_MAX_M              0.40      /* Z permitido durante swipe */

/* Posturas (mano izq para Pausa; ambos brazos para Salir) */
#define LIFT_ON_DELTA_M       0.05      /* activar: +5 cm sobre hombro */
#define LIFT_OFF_DELTA_M      0.02      /* liberar: +2 cm (histéresis) */

#define HOLD_PAUSE_MS         1500.0    /* 1.5 s para Pausa/Ready */
#define HOLD_QUIT_MS          1200.0    /* 1.2 s para Salir */

#define LOST_TOL_MS           700.0     /* tolerancia a pérdida breve */
#define REARM_NEUTRAL_MS      400.0     /* tiempo “fuera de postura” p/ rearmar */
#define MSG_DEBOUNCE_MS       600.0     /* evitar repetir “iniciada/cancelada” cada frame */

/* Suavizado (media móvil de Y y velocidad) */
#define SMOOTH_N              5         /* últimas N muestras */

/* Buffers y cola */
#define MAX_SAMPLES           64
#define QUE_MAX               32

typedef struct { HandSample s[MAX_SAMPLES]; int n; } SampleBuf;
static SampleBuf RBUF, LBUF;

static GestureParams GP;
static GestureEvent Q[QUE_MAX]; static int qh=0, qt=0;

/* Estado swipes (mano derecha) */
static int    armed = 0;
static double arm_start_ms=0;
static HandSample arm_ref={0};
static double last_ev_ms = 0;

/* Estado posturas */
static int    pause_active=0;      /* contando hold actual */
static double pause_start_ms=0;
static double pause_last_seen_ms=0;
static int    pause_rearm_wait=0;  /* esperando salir de postura para poder disparar otra vez */
static double pause_last_msg_ms=0; /* debounce de prints */

static int    quit_active=0;
static double quit_start_ms=0;
static double quit_last_seen_ms=0;
static int    quit_rearm_wait=0;
static double quit_last_msg_ms=0;

/* ==========================
   UTILIDADES
   ========================== */

static int q_push(GestureEvent e){ int nx=(qh+1)%QUE_MAX; if(nx==qt) return 0; Q[qh]=e; qh=nx; return 1; }
int gr_poll_event(GestureEvent* out){ if(qt==qh) return 0; *out=Q[qt]; qt=(qt+1)%QUE_MAX; return 1; }

static void push_sample(SampleBuf* B, const HandSample* h){
    if(!h) return;
    if(B->n<MAX_SAMPLES) B->s[B->n++]=*h;
    else { memmove(&B->s[0],&B->s[1],sizeof(B->s[0])*(MAX_SAMPLES-1)); B->s[MAX_SAMPLES-1]=*h; }
}
static int oldest_index_in_window(const SampleBuf* B, double window_ms){
    if(B->n==0) return -1; double t_now=B->s[B->n-1].t_ms; int i=B->n-1;
    while(i>=0 && (t_now - B->s[i].t_ms) <= window_ms) i--;
    return (i < B->n-1) ? (i+1) : (B->n-1);
}
static double avg_speed(const SampleBuf* B, int k){
    if(B->n<=k) return 0.0;
    const HandSample *a=&B->s[B->n-1-k], *b=&B->s[B->n-1];
    double dt=(b->t_ms-a->t_ms)/1000.0; if(dt<=0) return 0.0;
    double dx=b->x-a->x, dy=b->y-a->y, dz=b->z-a->z;
    return sqrt(dx*dx+dy*dy+dz*dz)/dt;
}
static double smooth_last_y(const SampleBuf* B){
    int n = (B->n<SMOOTH_N)? B->n : SMOOTH_N;
    if(n==0) return 0.0;
    double sum=0; int cnt=0;
    for(int i=0;i<n;i++){ if(B->s[B->n-1-i].visible){ sum+=B->s[B->n-1-i].y; cnt++; } }
    return (cnt>0)? (sum/cnt) : B->s[B->n-1].y;
}

/* ==========================
   ARMING (mano derecha)
   ========================== */

static void update_arming(const SampleBuf* B){
    if(B->n<2) return;
    const HandSample *a=&B->s[(B->n>=6)?B->n-6:0], *b=&B->s[B->n-1];

    double dt=(b->t_ms-a->t_ms)/1000.0; if(dt<=0) return;
    double dx=b->x-a->x, dy=b->y-a->y, dz=b->z-a->z;
    double speed=sqrt(dx*dx+dy*dy+dz*dz)/dt;
    double t=b->t_ms;

    if(!armed){
        if(arm_start_ms==0 || !arm_ref.visible){ arm_start_ms=t; arm_ref=*b; }
        double ddx=b->x-arm_ref.x, ddy=b->y-arm_ref.y, ddz=b->z-arm_ref.z;
        double drift=sqrt(ddx*ddx+ddy*ddy+ddz*ddz);
        if(speed<=STILL_SPEED_MAX_MPS && drift<=STILL_DRIFT_CM*0.01 && (t-arm_start_ms)>=ARM_DWELL_MS){
            armed=1;
        } else if(speed>STILL_SPEED_MAX_MPS || drift>STILL_DRIFT_CM*0.01){
            arm_start_ms=t; arm_ref=*b;
        }
    } else {
        if(speed > STILL_SPEED_MAX_MPS*1.6){ armed=0; arm_start_ms=t; arm_ref=*b; }
    }
}

/* ==========================
   SWIPES (mano derecha)
   ========================== */

static void try_detect_swipe(const SampleBuf* B){
    if(!armed) return;
    if(B->n<2) return;
    double t_now=B->s[B->n-1].t_ms;
    if(t_now - last_ev_ms < GP.cooldown_ms) return;

    int i0=oldest_index_in_window(B,(double)GP.swipe_window_ms);
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
    double dist=sqrt(dx*dx+dy*dy), speed=dist/dt;

    if (dist < dead_m) return;
    if (speed < vmin)  return;
    if (fabs(dz) > DZ_MAX_M) return;

    double adx=fabs(dx), ady=fabs(dy);
    double dir_ratio=(adx>ady)? adx/(ady+1e-6) : ady/(adx+1e-6);
    if(dir_ratio < DIR_RATIO_MIN) return;

    GestureEvent ev=GEV_NONE;
    if(adx>ady && adx>=swipe_m) ev = (dx>0)?GEV_RIGHT:GEV_LEFT;
    else if(ady>=swipe_m)      ev = (dy>0)?GEV_UP:GEV_DOWN;

    if(ev!=GEV_NONE){
        q_push(ev);
        last_ev_ms=t_now;
        armed=0; arm_start_ms=t_now; arm_ref=B->s[B->n-1]; /* evita contar el regreso */
    }
}

/* ==========================
   POSTURA: mano IZQUIERDA arriba -> Pausa/Ready
   ========================== */

static void detect_pause_left(const SampleBuf* L){
    if(L->n==0) return;
    const HandSample* cur=&L->s[L->n-1];
    double t=cur->t_ms;

    if(!cur->have_refs){ pause_active=0; pause_start_ms=0; return; }

    /* tolerancia a pérdidas breves */
    if(!cur->visible){
        if(pause_active) pause_last_seen_ms=t;
        if(!(pause_active && (t - pause_last_seen_ms) <= LOST_TOL_MS)){
            pause_active=0; pause_start_ms=0;
        }
        return;
    }

    /* suavizado de altura */
    double y_smooth = smooth_last_y(L);
    double above = y_smooth - cur->y_shoulder_L;  /* mano arriba => valor positivo */
    double v = avg_speed(L, (L->n>6)?6:(L->n-1));

    int posture_on  = (above >= LIFT_ON_DELTA_M)  && (v <= STILL_SPEED_MAX_MPS);
    int posture_off = (above <= LIFT_OFF_DELTA_M) || (v >  STILL_SPEED_MAX_MPS*1.3);

    if(pause_rearm_wait){
        if(posture_off && (pause_start_ms==0 || (t - pause_start_ms) >= REARM_NEUTRAL_MS)){
            pause_rearm_wait=0; pause_start_ms=0;
        }
        return;
    }

    if(!pause_active){
        if(posture_on){
            pause_active=1; pause_start_ms=t; pause_last_seen_ms=t;
            if(t - pause_last_msg_ms > MSG_DEBOUNCE_MS){ printf("[hold] Pausa iniciada…\n"); pause_last_msg_ms=t; }
        }
    }else{
        if(posture_off){
            pause_active=0; pause_start_ms=0;
            if(t - pause_last_msg_ms > MSG_DEBOUNCE_MS){ printf("[hold] Pausa cancelada\n"); pause_last_msg_ms=t; }
        }else{
            if((t - pause_start_ms) >= HOLD_PAUSE_MS){
                q_push(GEV_PAUSE_TOGGLE);
                pause_active=0; pause_start_ms=0;
                pause_rearm_wait=1;
                printf("[hold] Pausa/Ready OK\n");
                pause_last_msg_ms=t;
            }
        }
    }
}

/* ==========================
   POSTURA: AMBOS brazos arriba -> Salir
   ========================== */

static double smooth_last_y_of(const SampleBuf* B){ return smooth_last_y(B); }

static void detect_both_arms_up(const SampleBuf* R, const SampleBuf* L){
    if(R->n==0 || L->n==0) return;
    const HandSample *cr=&R->s[R->n-1], *cl=&L->s[L->n-1];
    double t=cr->t_ms;

    if(!(cr->have_refs && cl->have_refs)){ quit_active=0; quit_start_ms=0; return; }

    if(!(cr->visible && cl->visible)){
        if(quit_active) quit_last_seen_ms=t;
        if(!(quit_active && (t-quit_last_seen_ms) <= LOST_TOL_MS)){
            quit_active=0; quit_start_ms=0;
        }
        return;
    }

    /* suavizado altura por mano */
    double yR = smooth_last_y_of(R);
    double yL = smooth_last_y_of(L);

    double aboveR = yR - cr->y_shoulder_R;
    double aboveL = yL - cl->y_shoulder_L;

    double vR = avg_speed(R,(R->n>6)?6:(R->n-1));
    double vL = avg_speed(L,(L->n>6)?6:(L->n-1));

    int posture_on  = (aboveR >= LIFT_ON_DELTA_M) && (aboveL >= LIFT_ON_DELTA_M) &&
                      (vR <= STILL_SPEED_MAX_MPS) && (vL <= STILL_SPEED_MAX_MPS);
    int posture_off = (aboveR <= LIFT_OFF_DELTA_M) || (aboveL <= LIFT_OFF_DELTA_M) ||
                      (vR > STILL_SPEED_MAX_MPS*1.3) || (vL > STILL_SPEED_MAX_MPS*1.3);

    if(quit_rearm_wait){
        if(posture_off && (quit_start_ms==0 || (t - quit_start_ms) >= REARM_NEUTRAL_MS)){
            quit_rearm_wait=0; quit_start_ms=0;
        }
        return;
    }

    if(!quit_active){
        if(posture_on){
            quit_active=1; quit_start_ms=t; quit_last_seen_ms=t;
            if(t - quit_last_msg_ms > MSG_DEBOUNCE_MS){ printf("[hold] Salir iniciado…\n"); quit_last_msg_ms=t; }
        }
    }else{
        if(posture_off){
            quit_active=0; quit_start_ms=0;
            if(t - quit_last_msg_ms > MSG_DEBOUNCE_MS){ printf("[hold] Salir cancelado\n"); quit_last_msg_ms=t; }
        }else{
            if((t - quit_start_ms) >= HOLD_QUIT_MS){
                q_push(GEV_QUIT);
                quit_active=0; quit_start_ms=0;
                quit_rearm_wait=1;
                printf("[hold] Salir OK\n");
                quit_last_msg_ms=t;
            }
        }
    }
}

/* ==========================
   API
   ========================== */

void gr_init(const GestureParams* p){
    if(p) GP=*p; else memset(&GP,0,sizeof(GP));

    /* Defaults razonables si vienen en 0 */
    if(GP.dead_zone_cm<=0)       GP.dead_zone_cm=1;
    if(GP.swipe_threshold_cm<=0) GP.swipe_threshold_cm=5;
    if(GP.swipe_window_ms<=0)    GP.swipe_window_ms=500;
    if(GP.speed_min_cm_s<=0)     GP.speed_min_cm_s=10;
    if(GP.cooldown_ms<=0)        GP.cooldown_ms=300;

    RBUF.n=LBUF.n=0; qh=qt=0;
    armed=0; arm_start_ms=0; memset(&arm_ref,0,sizeof(arm_ref));
    last_ev_ms=0;

    pause_active=0; pause_start_ms=0; pause_rearm_wait=0; pause_last_seen_ms=0; pause_last_msg_ms=0;
    quit_active=0;  quit_start_ms=0;  quit_rearm_wait=0;  quit_last_seen_ms=0;  quit_last_msg_ms=0;
}

void gr_push_sample(const HandSample* right, const HandSample* left){
    push_sample(&RBUF,right);
    push_sample(&LBUF,left);

    detect_pause_left(&LBUF);
    detect_both_arms_up(&RBUF,&LBUF);

    update_arming(&RBUF);
    try_detect_swipe(&RBUF);
}

void gr_debug_inject(GestureEvent e){ q_push(e); }
