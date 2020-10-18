#include "cube.h"

// Main state.
// cube[z][x] = y (binary encoded).
uint8_t cube[8][8];

#define writePulse(value)      \
    digitalWrite(MOSI, value); \
    digitalWrite(SCK, LOW);    \
    digitalWrite(SCK, HIGH);

#define setVoxel(x, y, z)                      \
    if (x % 2 == 0) {                          \
        cube[7 - z][x + 1] |= 0x01 << (7 - y); \
    } else {                                   \
        cube[7 - z][x - 1] |= 0x01 << y;       \
    }

void voxelExplorer() {
    static int x = 0;
    static int y = 0;
    static int z = 0;
    static uint16_t timer = 0;

    timer++;

    clearCube();
    setVoxel(x, y, z);

    if (timer < 50) {
        return;
    }

    timer = 0;
    if (++x >= 8) {
        x = 0;
        y++;
    }
    if (y >= 8) {
        y = 0;
        z++;
    }
    if (z >= 8) {
        z = 0;
    }
}

void loopCube() {
    rain();
    renderCube();
}

void setupCube() {
    // Enable serial for debug.
    Serial.begin(9600);
    Serial.println();

    // Enable the SPI pins.
    pinMode(SS, OUTPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(SCK, OUTPUT);

    // Initilize the cube.
    clearCube();
}

void renderCube() {
    for (uint8_t z = 0; z < 8; z++) {
        digitalWrite(SS, LOW);

        shiftOut(MOSI, SCK, MSBFIRST, 0x01 << z);
        // for (int i = 0; i < 8; i++) {
        //     writePulse(i == z ? HIGH : LOW);
        // }
        for (int x = 0; x < 8; x++) {
            // for (int y = 0; y < 8; y++) {
            //     writePulse(cube[z][x] & (1 << y) ? HIGH : LOW);
            // }
            shiftOut(MOSI, SCK, MSBFIRST, cube[z][x]);
        }

        digitalWrite(SS, HIGH);
    }
}

void fill(int z, int value) {
    for (int x = 0; x < 8; x++) {
        cube[z][x] = value;
    }
}

void clearCube() {
    for (int z = 0; z < 8; z++) {
        fill(z, 0);
    }
}

// ASYNC_DELAY returns the caller function until the specified
// interval (in ms) has past.
#define ASYNC_DELAY(x)               \
    static unsigned long __last = 0; \
    unsigned long __cur = millis();  \
    if (__cur - __last < x) return;  \
    __last = __cur;

void rain() {
    ASYNC_DELAY(RAIN_SPEED);

    // Shift the cube on the Z axis, i.e. make the droplets "fall".
    shift(NEG_Z);

    // Generate net random droplets for the top layer.
    uint8_t numDrops = random(0, 5);
    for (uint8_t i = 0; i < numDrops; i++) {
        setVoxel(random(0, 8), random(0, 8), 0);
    }
}

void shift(int dir) {
    switch (dir) {
        case NEG_Z:
            for (int z = 0; z < 7; z++) {
                for (int x = 0; x < 8; x++) {
                    cube[z][x] = cube[z + 1][x];
                }
            }
            // Clear out the first layer.
            fill(7, 0);
            break;
            // TODO: Implement the rest of the shifts.
    }
}
