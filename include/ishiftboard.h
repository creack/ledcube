#pragma once

#include <Arduino.h>

#include "iboard.h"

// IShiftBoard is an abstract class implementing most of the common shift register based board in LSB.
// The actual shift() method needs to be implemented by the concrete class.
class IShiftBoard : public IBoard {
   public:
    IShiftBoard(int clockPin, int latchPin, int dataPin, int byteOrder = LSBFIRST) {
        this->_clockPin = clockPin;
        this->_latchPin = latchPin;
        this->_dataPin = dataPin;
        this->_byteOrder = byteOrder;
    }

    void setup() const {
        ::pinMode(this->_clockPin, OUTPUT);
        ::pinMode(this->_latchPin, OUTPUT);
        ::pinMode(this->_dataPin, OUTPUT);
    }

    void render(const ICubeRO& cube) const {
        for (int z = 0; z < 8; z++) {
            this->latch();

            this->shift(0x01 << z);

            for (int x = 0; x < 8; x++) {
                uint8_t tmp = 0;
                for (int y = 0; y < 8; y++) {
                    tmp |= this->getVoxel(cube, x, y, z) << y;
                }
                this->shift(tmp);
            }

            this->unlatch();
        }
    }

    virtual void shift(uint8_t line) const = 0;

   protected:
    void latch() const {
        ::digitalWrite(this->_latchPin, LOW);
    }

    void unlatch() const {
        ::digitalWrite(this->_latchPin, HIGH);
    }

   protected:
    int _clockPin;  // aka SCK.
    int _latchPin;  // aka SS.
    int _dataPin;   // aka MOSI.
    int _byteOrder;
};
