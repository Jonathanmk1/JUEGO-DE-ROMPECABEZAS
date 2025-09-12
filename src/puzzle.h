#ifndef PUZZLE_H
#define PUZZLE_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    int N;
    int tiles[4][4];
    int empty_r, empty_c;
    int moves_count;
    int is_solved;
} Puzzle;
typedef enum { DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN } MoveDir;
void pu_init(Puzzle* pu, int N);
void pu_shuffle(Puzzle* pu, unsigned seed, int steps);
int  pu_can_move(const Puzzle* pu, MoveDir d);
int  pu_move(Puzzle* pu, MoveDir d);
int  pu_check_solved(Puzzle* pu);
#ifdef __cplusplus
}
#endif
#endif