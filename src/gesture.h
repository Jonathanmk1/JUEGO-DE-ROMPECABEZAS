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
    double x, y, z;
    double t_ms;
    int    visible;

    double y_shoulder_L;
    double y_shoulder_R;
    double y_head;
    int    have_refs;
} HandSample;

void gr_init(const GestureParams* p);
void gr_push_sample(const HandSample* right, const HandSample* left);
int  gr_poll_event(GestureEvent* out_event);
void gr_debug_inject(GestureEvent e);

#endif
