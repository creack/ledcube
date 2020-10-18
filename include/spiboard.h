#pragma once

#include <SPI.h>

#include "ishiftboard.h"

class SPIBoard : public IShiftBoard {
   public:
    SPIBoard(int clockPin, int latchPin, int dataPin, int byteOrder = LSBFIRST, uint32_t speed = 800000, int mode = SPI_MODE0);
    void setup() const;

   private:
    void shift(uint8_t line) const;

   private:
    uint32_t _speed;
    int _mode;
};
