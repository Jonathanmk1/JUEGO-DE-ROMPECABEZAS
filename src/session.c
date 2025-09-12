#include "session.h"
void se_init(Session* s,int N){s->st=ST_READY;s->t_start_s=0.0;s->t_paused_s=0.0;s->difficulty_N=(N==4)?4:3;}
void se_handle_event(Session* s,GestureEvent ev,Puzzle* pu){
    switch(s->st){
        case ST_READY: if(ev==GEV_PAUSE_TOGGLE){s->st=ST_PLAY;} break;
        case ST_PLAY:
            if(ev==GEV_LEFT) pu_move(pu,DIR_LEFT);
            if(ev==GEV_RIGHT)pu_move(pu,DIR_RIGHT);
            if(ev==GEV_UP)   pu_move(pu,DIR_UP);
            if(ev==GEV_DOWN) pu_move(pu,DIR_DOWN);
            if(ev==GEV_PAUSE_TOGGLE) s->st=ST_PAUSE;
            if(ev==GEV_MIX) s->st=ST_MIX; break;
        case ST_PAUSE: if(ev==GEV_PAUSE_TOGGLE) s->st=ST_PLAY; break;
        case ST_MIX: break;
    }
}