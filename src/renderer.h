#ifndef RENDERER_H
#define RENDERER_H
#include "puzzle.h"
#include "gesture.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {int width,height;const char* hud_lang;int show_hand_cursor;int show_feedback_arrows;} RenderConfig;
typedef struct {void* _impl;} RenderCtx;
int re_init(RenderCtx* rc,const RenderConfig* cfg);
void re_shutdown(RenderCtx* rc);
int re_load_image(RenderCtx* rc,const char* path,int N);
void re_draw(const RenderCtx* rc,const Puzzle* pu,const HandSample* right,int state_ready,int state_pause);
void re_show_victory(const RenderCtx* rc,int moves,double time_s);
#ifdef __cplusplus
}
#endif
#endif