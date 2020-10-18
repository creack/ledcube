#ifndef __CUBE_H__
#define __CUBE_H__

#include <Arduino.h>

#define POS_X 0
#define NEG_X 1
#define POS_Z 2
#define NEG_Z 3
#define POS_Y 4
#define NEG_Y 5

/* Maximum number of droplets per layer. */
#define RAIN_MAX_DROPLETS
/* Time for the rain droplets to fall one layer, in ms. */
#define RAIN_SPEED 100

void setVoxel(int x, int y, int z);
void shift(int);
void renderCube();
void rain();
void clearCube();

extern uint8_t cube[8][8];

#endif /* !__CUBE_H__ */
