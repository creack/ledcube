#include <Arduino.h>

void setupCube();
void setupWiFi();
void loopCube();
void loopWiFi();

void setup() {
    // Enable serial for debug.
    Serial.begin(9600);
    Serial.println();

    setupCube();
    // setupWiFi();
}

void loop() {
    loopCube();
    // loopWiFi();
}
