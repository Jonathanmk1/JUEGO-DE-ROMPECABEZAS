#include "puzzle.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void set_solved(Puzzle* pu){
    for (int r=0;r<pu->N;r++){
        for (int c=0;c<pu->N;c++){
            int val=r*pu->N+c+1;
            if(r==pu->N-1 && c==pu->N-1) val=0;
            pu->tiles[r][c]=val;
        }
    }
    pu->empty_r=pu->N-1;
    pu->empty_c=pu->N-1;
    pu->moves_count=0;
    pu->is_solved=1;
}
void pu_init(Puzzle* pu, int N){
    if(!pu) return;
    if(N!=3 && N!=4) N=3;
    memset(pu,0,sizeof(*pu));
    pu->N=N;
    set_solved(pu);
}
int pu_can_move(const Puzzle* pu, MoveDir d){
    int r=pu->empty_r,c=pu->empty_c;
    switch(d){
        case DIR_LEFT: return (c+1)<pu->N;
        case DIR_RIGHT:return (c-1)>=0;
        case DIR_UP:   return (r+1)<pu->N;
        case DIR_DOWN: return (r-1)>=0;
        default: return 0;
    }
}
static void swap(int* a,int* b){int t=*a;*a=*b;*b=t;}
int pu_move(Puzzle* pu, MoveDir d){
    if(!pu_can_move(pu,d)) return 0;
    int r=pu->empty_r,c=pu->empty_c,rr=r,cc=c;
    if(d==DIR_LEFT) cc=c+1;
    if(d==DIR_RIGHT)cc=c-1;
    if(d==DIR_UP)   rr=r+1;
    if(d==DIR_DOWN) rr=r-1;
    swap(&pu->tiles[r][c],&pu->tiles[rr][cc]);
    pu->empty_r=rr;pu->empty_c=cc;
    pu->moves_count++;
    pu_check_solved(pu);
    return 1;
}
int pu_check_solved(Puzzle* pu){
    int k=1;
    for(int r=0;r<pu->N;r++){
        for(int c=0;c<pu->N;c++){
            int expected=(r==pu->N-1 && c==pu->N-1)?0:k;
            if(pu->tiles[r][c]!=expected){pu->is_solved=0;return 0;}
            if(expected) k++;
        }
    }
    pu->is_solved=1;
    return 1;
}
void pu_shuffle(Puzzle* pu, unsigned seed,int steps){
    if(seed==0) seed=(unsigned)time(NULL);
    srand(seed);
    for(int i=0;i<steps;i++){
        MoveDir cand[4];int n=0;
        if(pu_can_move(pu,DIR_LEFT)) cand[n++]=DIR_LEFT;
        if(pu_can_move(pu,DIR_RIGHT))cand[n++]=DIR_RIGHT;
        if(pu_can_move(pu,DIR_UP))   cand[n++]=DIR_UP;
        if(pu_can_move(pu,DIR_DOWN)) cand[n++]=DIR_DOWN;
        if(n>0) pu_move(pu,cand[rand()%n]);
    }
    pu->moves_count=0;
    pu_check_solved(pu);
}