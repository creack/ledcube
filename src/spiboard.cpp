#include "spiboard.h"

SPIBoard::SPIBoard(int clockPin, int latchPin, int dataPin, int byteOrder, uint32_t speed, int mode) : IShiftBoard(clockPin, latchPin, dataPin, byteOrder),
                                                                                                       _speed(speed),
                                                                                                       _mode(mode) {
}

void SPIBoard::setup() const {
    ::SPI.begin();
    ::SPI.beginTransaction(::SPISettings(this->_speed, this->_byteOrder, this->_mode));
}

void SPIBoard::shift(uint8_t line) const {
    ::SPI.transfer(line);
}
