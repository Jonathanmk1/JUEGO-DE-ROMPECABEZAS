#ifndef CONFIG_H
#define CONFIG_H
#include "gesture.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    char dominant_hand[8];
    int use_left_precision;
    GestureParams gp;
    float distance_to_camera_m;
    float hand_height_ref_cm;
    float hand_size_scale;
    int show_hand_cursor;
    int show_feedback_arrows;
    char hud_language[4];
} AppConfig;
int cfg_load(AppConfig* out,const char* path);
int cfg_save(const AppConfig* in,const char* path);
#ifdef __cplusplus
}
#endif
#endif