#include "config.h"
#include <string.h>

static void defaults(AppConfig* c){
    memset(c,0,sizeof(*c));
    strcpy(c->dominant_hand,"right");
    c->use_left_precision=0;

    /* Swipes más fáciles */
    c->gp.dead_zone_cm       = 1;
    c->gp.swipe_threshold_cm = 5;
    c->gp.swipe_window_ms    = 500;
    c->gp.pause_hold_ms      = 1000;
    c->gp.double_tap_gap_ms  = 500;
    c->gp.speed_min_cm_s     = 10;   /* 0.10 m/s */
    c->gp.cooldown_ms        = 300;
    c->gp.use_left_precision = 0;

    c->distance_to_camera_m  = 1.8f;
    c->hand_height_ref_cm    = 120.0f;
    c->hand_size_scale       = 1.0f;
    c->show_hand_cursor      = 1;
    c->show_feedback_arrows  = 1;
    strcpy(c->hud_language,"es");
}
int cfg_load(AppConfig* out,const char* path){ (void)path; defaults(out); return 0; }
int cfg_save(const AppConfig* in,const char* path){ (void)in; (void)path; return 0; }
