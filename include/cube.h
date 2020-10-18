#ifndef __CUBE_H__
#define __CUBE_H__

#include <Arduino.h>

#define writePulse(value)      \
    digitalWrite(MOSI, value); \
    digitalWrite(SCK, LOW);    \
    digitalWrite(SCK, HIGH);

// ASYNC_DELAY returns the caller function until the specified
// interval (in ms) has past.
#define ASYNC_DELAY(x)               \
    static unsigned long __last = 0; \
    unsigned long __cur = millis();  \
    if (__cur - __last < x) return;  \
    __last = __cur;

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
void clearCube();

// Effects.
void rain();
void voxelExplorer();

extern uint8_t cube[8][8];

class Cube {
   private:
    uint8_t _state[8][8];

   public:
    Cube();
};

#endif /* !__CUBE_H__ */
