#include "shiftpulseboard.h"

void ShiftPulseBoard::shift(uint8_t line) const {
    ::shiftOut(this->_dataPin, this->_clockPin, this->_byteOrder, line);
}
