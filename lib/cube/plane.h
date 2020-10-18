
#pragma once

class Plane {
   public:
    enum axis {
        x = 1,
        y = 2,
        z = 3,
    };

    enum direction {
        stale = 0,
        positive = 1,
        negative = -1,
    };

    static Plane X, Y, Z;

    Plane(axis axis = axis::x, int offset = 0, direction direction = direction::stale) : _axis(axis),
                                                                                         _offset(offset),
                                                                                         _direction(direction) {
    }

    // operator int exposes the underlying offset when expecting an int type.
    // ex:
    //   void printOffset(int offset) { std::cout << offset << std::endl; }
    //   int main() {
    //     Plane p = Plane(Plane::axis::y, 6);
    //     printOffset(p); // prints "6".
    //    }
    operator int() const { return this->_offset; }

    // operator axis exposes the underlying offset when expecting an axis type.
    // ex:
    //   void printAxis(Plane::axis axis) { std::cout << axis << std::endl; }
    //   int main() {
    //     Plane p = Plane(Plane::axis::z);
    //     printAxis(p); // prints "2".
    //     if (p == Plane::axis::z) {
    //       std::cout << "It also works with conditions." << std::endl;
    //     }
    //   }
    operator axis() const { return this->_axis; }

    // operator direction exposes the underlying offset when expecting a direction type.
    // ex:
    //   void printDirection(Plane::direction dir) { std::cout << dir << std::endl; }
    //   int main() {
    //     foo(-Plane::X); // prints "-1".
    //     foo(+Plane::X); // prints "-1".
    //     foo(Plane::X);  // prints "0".
    //   }
    operator direction() const { return this->_direction; }

    // operator()(int) sets the given offset component.
    // ex:
    //   Plane p = Plane::X(5);
    //   (int)p == 5; // true.
    Plane operator()(int offset) const {
        return Plane(this->_axis, offset, this->_direction);
    }

    // operator+ sets the direction of the plane to positive.
    // ex:
    //    Plane p(Plane::axis::x, 0, Plane::direction::stale);
    //    Plane p1 = +p;
    Plane operator+() const {
        return Plane(this->_axis, this->_offset, direction::positive);
    }

    // operator- sets the direction of the plane.
    // ex:
    //   Plane p(Plane::axis::x, 0, Plane::direction::stale);
    //   Plane p1 = -p;
    Plane operator-() const {
        return Plane(this->_axis, this->_offset, direction::negative);
    }

    // operator! is used to reverse the direction of the plane.
    // No-op when the direction is stale.
    // ex:
    //   Plane p = -Plane::X;
    //   (Plane::direction)p == -1 // true.
    //   p = !p;
    //   (Plane::direction)p == 1 // true.
    Plane operator!() const {
        return Plane(this->_axis, this->_offset, (direction)(-this->_direction));
    }

    // operator++ updtes the offset based on the direction.
    // TODO: As this can increment as well as decrement, might want to consider a different operator.
    // ex:
    //   Plane p = +Plane::Y(4);
    //   ++p;
    //   (int)p == 5 // true.
    //
    //   Plane p = Plane::Z(1);
    //   ++p; // no-op as p is in stale direction.
    //   (int)p == 1 // true.
    //
    //   Plane p = -Plane::X(0);
    //   ++p;
    //   (int)p == -1; // true.
    Plane& operator++() {
        this->_offset += this->_direction;
        return *this;
    }

   private:
    axis _axis;
    int _offset;
    direction _direction;
};
