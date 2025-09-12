#ifndef KINECT_INPUT_H
#define KINECT_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gesture.h"

/* Inicializa Kinect v1 (SDK 1.8). Devuelve 0 si OK. */
int  ki_init(void);

/* Cierra el dispositivo */
void ki_shutdown(void);

/* Lee manos (prioriza mano derecha del primer esqueleto TRACKED).
   Devuelve 0 si hay frame; -1 si timeout. */
int  ki_read_hands(HandSample* right, HandSample* left);

#ifdef __cplusplus
}
#endif
#endif