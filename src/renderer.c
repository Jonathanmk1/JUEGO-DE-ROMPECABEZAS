#include "renderer.h"
#include <stdio.h>
int re_init(RenderCtx* rc,const RenderConfig* cfg){(void)rc;(void)cfg;printf("[renderer] init (stub)\n");return 0;}
void re_shutdown(RenderCtx* rc){(void)rc;}
int re_load_image(RenderCtx* rc,const char* path,int N){(void)rc;(void)path;(void)N;printf("[renderer] load image (stub)\n");return 0;}
static void print_board(const Puzzle* pu){for(int r=0;r<pu->N;r++){for(int c=0;c<pu->N;c++){int v=pu->tiles[r][c];if(v==0)printf("  . ");else printf("%3d ",v);}printf("\n");}}
void re_draw(const RenderCtx* rc,const Puzzle* pu,const HandSample* right,int state_ready,int state_pause){
    (void)rc;(void)right;
    printf("\n=== TABLERO (%dx%d) ===%s%s\n",pu->N,pu->N,state_ready?" [READY]":"",state_pause?" [PAUSE]":"");
    print_board(pu);
    printf("Movs: %d   %s\n",pu->moves_count,pu->is_solved?"(RESUELTO)":"");
}
void re_show_victory(const RenderCtx* rc,int moves,double time_s){
    (void)rc;
    printf("\n*** ¡COMPLETADO! *** Movimientos: %d  Tiempo: %.1fs\n",moves,time_s);
}