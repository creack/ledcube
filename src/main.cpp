#include "cube.h"
#include "effects.h"
#include "iboard.h"
#include "shiftpulseboard.h"
#include "spiboard.h"

// overrides getVoxel to match the physical wiring of the cube controller.
int getVoxel(const ICubeRO& cube, int x, int y, int z) {
    if (x % 2 == 0) {
        return cube.getVoxel(x + 1, y, 7 - z);
    }
    return cube.getVoxel(x - 1, 7 - y, 7 - z);
}

IBoard* const boards[] = {
    // new SPIBoard(SCK, SS, MOSI),
    new ShiftPulseBoard(SCK, SS, MOSI),
};
IBoard* board = boards[0];

Cube cube;

IEffect* effects[] = {
    // new Rain(100, 5, +Plane::X),
    // new Rain(100, 5, -Plane::X),
    // new Rain(100, 5, +Plane::Y),
    // new Rain(100, 5, -Plane::Y),
    // new Rain(100, 5, +Plane::Z),
    new Rain(100, 5, -Plane::Z),

    // new SendVoxels(50, Plane::X),
    // new SendVoxels(50, Plane::Y),
    new SendVoxels(50, Plane::Z),

    new VoxelExplorer(100),
    new PlaneBoing(100),
    new FullyOn(),
    new WoopWoop(100),
    new CubeJump(50),
    new Glowing(),
    new Numbers(100, +Plane::Y),

    0,
};

EffectCycler cycler(10000, effects);

void setup() {
    Serial.begin(9600);
    Serial.println("");
    Serial.println("Starting!");

    randomSeed(analogRead(0));
    board->setup();
    board->setMapping(getVoxel);

    //cycler = new Numbers(1000, -Plane::Y);
    cycler = new Glowing();
    //cycler = cycler[0];
    cycler.current()->init(cube);
}

void loop() {
    unsigned long currentTime = ::millis();

    cycler.loop(currentTime, cube);
    cycler.current()->loop(currentTime, cube);

    board->render(cube);
}
