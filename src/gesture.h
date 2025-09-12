#ifndef GESTURE_H
#define GESTURE_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {float x,y,z;int visible;double t_ms;} HandSample;
typedef enum {GEV_NONE=0,GEV_LEFT,GEV_RIGHT,GEV_UP,GEV_DOWN,GEV_PAUSE_TOGGLE,GEV_MIX} GestureEvent;
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
void gr_init(const GestureParams* p);
void gr_push_sample(const HandSample* right,const HandSample* left);
int gr_poll_event(GestureEvent* out_event);
void gr_debug_inject(GestureEvent e);
#ifdef __cplusplus
}
#endif
#endif