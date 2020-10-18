#pragma once

#include "ishiftboard.h"

class ShiftPulseBoard : public IShiftBoard {
    // Inherit the base constructor.
    using IShiftBoard::IShiftBoard;

   private:
    void shift(uint8_t line) const;
};
