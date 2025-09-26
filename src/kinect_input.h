#ifndef KINECT_INPUT_H
#define KINECT_INPUT_H

#include "gesture.h"  /* para HandSample */

int  ki_init(void);
void ki_shutdown(void);

/* lee manos derecha/izquierda y completa referencias (y hombros/cabeza) */
void ki_read_hands(HandSample* right, HandSample* left);

#endif
