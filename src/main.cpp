#include <Arduino.h>

void setupCube();
void setupWiFi();
void loopCube();
void loopWiFi();

void setup() {
    setupCube();
    // setupWiFi();
}

void loop() {
    loopCube();
    // loopWiFi();
}
