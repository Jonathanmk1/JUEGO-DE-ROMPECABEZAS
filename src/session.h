#ifndef SESSION_H
#define SESSION_H
#include "puzzle.h"
#include "gesture.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {ST_READY,ST_PLAY,ST_PAUSE,ST_MIX} GameState;
typedef struct {GameState st;double t_start_s;double t_paused_s;int difficulty_N;} Session;
void se_init(Session* s,int N);
void se_handle_event(Session* s,GestureEvent ev,Puzzle* pu);
#ifdef __cplusplus
}
#endif
#endif