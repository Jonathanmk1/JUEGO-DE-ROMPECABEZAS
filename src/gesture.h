#ifndef GESTURE_H
#define GESTURE_H

typedef enum {
    GEV_NONE = 0,
    GEV_LEFT,
    GEV_RIGHT,
    GEV_UP,
    GEV_DOWN,
    GEV_MIX,
    GEV_PAUSE_TOGGLE,
    GEV_QUIT
} GestureEvent;

typedef struct {
    int dead_zone_cm;
    int swipe_threshold_cm;
    int swipe_window_ms;
    int pause_hold_ms;
    int double_tap_gap_ms;
    int speed_min_cm_s;
    int cooldown_ms;
    int use_left_precision;
} GestureParams;

typedef struct {
    double x, y, z;   /* metros, coords cámara */
    double t_ms;      /* timestamp ms */
    int visible;      /* 1 si visible */
} HandSample;

void gr_init(const GestureParams* p);
void gr_push_sample(const HandSample* right, const HandSample* left);
int  gr_poll_event(GestureEvent* out_event);

/* debug (teclado) */
void gr_debug_inject(GestureEvent e);

#endif
