#include <Arduino.h>

#include "cube.h"

class IEffect {
   public:
    virtual void init(ICube& cube) {}

    virtual bool ready(unsigned long currentTime) = 0;
    virtual void step(ICube& cube) = 0;

    virtual void loop(unsigned long currentTime, ICube& cube) {
        if (this->ready(currentTime)) {
            this->step(cube);
        }
    }
};

class BaseEffect : public IEffect {
   public:
    BaseEffect(unsigned int stepInterval) {
        this->_interval = stepInterval;
        this->_last = 0;
    }

    bool ready(unsigned long currentTime) {
        if (currentTime - this->_last < (unsigned int)this->_interval) {
            return false;
        }
        this->_last = currentTime;
        return true;
    }

    virtual void step(ICube& cube) = 0;

   private:
    unsigned int _interval;
    unsigned long _last;
};

class EffectCycler : public BaseEffect {
   public:
    enum mode {
        sequence,
        random,
        fixed,
    };

    // Expect null terminated effects list.
    EffectCycler(unsigned int speed, IEffect* effects[], mode mode = mode::sequence) : BaseEffect(speed),
                                                                                       _cycleMode(mode),
                                                                                       _effects(effects) {
        // Lookup the list size.
        for (this->_size = 0; effects[this->_size]; ++this->_size) {
        };
    }

    EffectCycler(IEffect* effect) : BaseEffect(1000) {
        this->_effects = &this->_placeHolder;
        *this = effect;
    }

    void step(ICube& cube) {
        switch (this->_cycleMode) {
            case mode::fixed:
                return;
            case mode::random:
                this->_idx = ::random(0, this->_size);
                break;
            case mode::sequence:
            default:
                this->_idx = (this->_idx + 1) % this->_size;
                break;
        }
        Serial.print("Now using effect #");
        Serial.print(this->_idx);
        Serial.print(" (size: ");
        Serial.print(this->_size);
        Serial.println(")");
        // Clear the cube and call the effect init.
        cube.clear();
        this->current()->init(cube);
    }

    IEffect* current() const {
        return this->_effects[this->_idx];
    }

    EffectCycler& operator=(int idx) {
        this->_idx = idx;
        return *this;
    }

    EffectCycler& operator=(mode mode) {
        this->_cycleMode = mode;
        return *this;
    }

    EffectCycler& operator=(IEffect* effect) {
        this->_idx = 0;
        this->_size = 1;
        this->_cycleMode = mode::fixed;
        this->_effects[0] = effect;
        return *this;
    }

    IEffect* operator[](int idx) {
        return this->_effects[idx];
    }

   private:
    mode _cycleMode = mode::sequence;
    int _idx = 0;
    int _size;
    IEffect** _effects;
    IEffect* _placeHolder;
};

// VoxelExplorer is mostly to test the wiring of the cube.
// Turn on a single LED, start at 0,0,0 and explore the whoel cube.
class VoxelExplorer : public BaseEffect {
   public:
    VoxelExplorer(unsigned int speed) : BaseEffect(speed) {}

    void step(ICube& cube) {
        cube.clear();
        cube.setVoxel(this->_x, this->_y, this->_z, 1);

        if (++this->_x >= 8) {
            this->_x = 0;
            this->_y++;
        }
        if (this->_y >= 8) {
            this->_y = 0;
            this->_z++;
        }
        if (this->_z >= 8) {
            this->_z = 0;
        }
    }

   private:
    int _x = 0;
    int _y = 0;
    int _z = 0;
};

// Rain creates a rain effect:
// - Turn on a random number of leds (up to maxDroplets) on the first layer (based on plane direction).
// - Shift the cube based on the plane direction so the previously lit leds "fall" to the next layer.
// - Reset the first layer.
// - Repeat.
class Rain : public BaseEffect {
   public:
    Rain(unsigned int speed, unsigned int maxDroplets = 5, const Plane& p = -Plane::Z) : BaseEffect(speed),
                                                                                         _maxDroplets(maxDroplets),
                                                                                         _plane(p) {
    }

    void step(ICube& cube) {
        // Shift the whole cube along the plane.
        cube.shift(this->_plane);

        // Populate the first or last layer based on the the plane direction.
        int offset = this->_plane == Plane::direction::positive ? 0 : 7;

        // Generate random droplets for the first/last layer.
        int numDrops = ::random(0, this->_maxDroplets);
        for (int i = 0; i < numDrops; i++) {
            cube.setVoxel(this->_plane(offset), random(0, 8), random(0, 8), 1);
        }
    }

   private:
    unsigned int _maxDroplets;
    const Plane _plane;
};

// PlaneBoing lights a single layer plane and moves it accross the cube.
// - Start with a fully lit layer.
// - Move up until the end.
// - Move down back to 0.
// - Pick next axis.
// - Repeat.
class PlaneBoing : public BaseEffect {
   public:
    PlaneBoing(unsigned long speed) : BaseEffect(speed) {}

    void init(ICube& cube) {
        // Start with the selected plane at offset 0.
        cube.fill(this->_planes[this->_currentPlane](0), 1);
    }

    void step(ICube& cube) {
        Plane& p = this->_planes[this->_currentPlane];

        // Shift the whole cube (i.e. a single plane).
        cube.shift(p);

        // Update the offset of the plane.
        ++p;

        if ((int)p >= 7) {
            // If we are at the last positive offset, reverse the direction.
            p = !p;
        } else if ((int)p < 0) {
            // If we are at the last negative offset, reset the plane object,
            p = +p(0);
            // then move to the next plane,
            this->_currentPlane = (this->_currentPlane + 1) % 3;
            // and lit the first layer.
            cube.fill(this->_planes[this->_currentPlane](0), 1);
        }
    }

   private:
    int _currentPlane = 0;
    Plane _planes[3] = {+Plane::X, +Plane::Y, +Plane::Z};
};

// SendVoxel lights random LEDs on 2 opposite layer planes (first and last) then "moves"
// LEDs randomly, one by one along the axis, lightning up each LED on the way.
class SendVoxels : public BaseEffect {
   public:
    SendVoxels(unsigned long speed, Plane::axis axis = Plane::Z) : BaseEffect(speed), _plane(axis) {}

    void init(ICube& cube) {
        // Start by lightning up a layer and randomly spread it among the edges.
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                // Randomly pick the first or last layer.
                int k = ::random(0, 2) % 2 ? 0 : 7;

                cube.setVoxel(this->_plane(k), i, j, 1);
            }
        }
    }

    void step(ICube& cube) {
        // If we are not currently sending a voxel, pick one to send.
        if (!this->_sending) {
            // Pick a random point in the plane.
            this->_i = ::random(0, 8);
            this->_j = ::random(0, 8);

            // Check if the layer 0 is lit.
            if (cube.getVoxel(this->_plane(0), this->_i, this->_j)) {
                this->_plane = +this->_plane(0);
            } else {
                this->_plane = -this->_plane(7);
            }

            this->_sending = true;
            return;
        }

        // If we are sending, turn off the current LED and light up the next one.
        cube.setVoxel(this->_plane, this->_i, this->_j, 0);
        ++this->_plane;
        cube.setVoxel(this->_plane, this->_i, this->_j, 1);

        // When we reach the first or last layer, reset the sending flag and start again.
        if ((int)this->_plane <= 0 || (int)this->_plane >= 7) {
            this->_sending = false;
        }
    }

   private:
    int _i;
    int _j;
    Plane _plane;
    bool _sending = false;
};

class FullyOn : public BaseEffect {
   public:
    FullyOn() : BaseEffect(1000) {}

    void init(ICube& cube) {
        for (int i = 0; i < 8; i++) {
            cube.fill(Plane::Z(i), 1);
        }
    }

    void step(ICube& cube) {}
};

void drawCube(ICube& cube, int x, int y, int z, int size) {
    for (int i = 0; i < size; i++) {
        cube.setVoxel(x, y + i, z, 1);
        cube.setVoxel(x + i, y, z, 1);
        cube.setVoxel(x, y, z + i, 1);
        cube.setVoxel(x + size - 1, y + i, z + size - 1, 1);
        cube.setVoxel(x + i, y + size - 1, z + size - 1, 1);
        cube.setVoxel(x + size - 1, y + size - 1, z + i, 1);
        cube.setVoxel(x + size - 1, y + i, z, 1);
        cube.setVoxel(x, y + i, z + size - 1, 1);
        cube.setVoxel(x + i, y + size - 1, z, 1);
        cube.setVoxel(x + i, y, z + size - 1, 1);
        cube.setVoxel(x + size - 1, y, z + i, 1);
        cube.setVoxel(x, y + size - 1, z + i, 1);
    }
}

// WoopWoop draws a cube in the center, growing and shrinking betwen 2x2x2 and 8x8x8 (lightning only the edges).
class WoopWoop : public BaseEffect {
   public:
    WoopWoop(unsigned int speed) : BaseEffect(speed) {}

    void step(ICube& cube) {
        if (this->_expanding) {
            // If we are in the expanding step, keep growing.
            this->_size += 2;
            // If we reach the max size, stop.
            if (this->_size == 8) {
                this->_expanding = false;
            }
        } else {
            // If we are not expanding, keep shrinking.
            this->_size -= 2;
            if (this->_size == 2) {
                // When we reach the min size, stop.
                this->_expanding = true;
            }
        }
        // Clean the state and draw the new sized cube.
        cube.clear();
        drawCube(cube, 4 - this->_size / 2, 4 - this->_size / 2, 4 - this->_size / 2, this->_size);
    }

   private:
    int _size = 2;
    bool _expanding = true;
};

// CubeJump draws a cube in a corner, grows it until it reach the max size, then start again from a different corner.
class CubeJump : public BaseEffect {
   public:
    CubeJump(unsigned int speed) : BaseEffect(speed) {}

    void init(ICube& cube) {
        this->_xPos = ::random(0, 2) * 7;
        this->_yPos = ::random(0, 2) * 7;
        this->_zPos = ::random(0, 2) * 7;
        this->_size = 8;
        this->_expanding = false;
    }

    void step(ICube& cube) {
        cube.clear();

        if (this->_xPos == 0 && this->_yPos == 0 && this->_zPos == 0) {
            drawCube(cube, this->_xPos, this->_yPos, this->_zPos, this->_size);
        } else if (this->_xPos == 7 && this->_yPos == 7 && this->_zPos == 7) {
            drawCube(cube, this->_xPos + 1 - this->_size, this->_yPos + 1 - this->_size, this->_zPos + 1 - this->_size, this->_size);
        } else if (this->_xPos == 7 && this->_yPos == 0 && this->_zPos == 0) {
            drawCube(cube, this->_xPos + 1 - this->_size, this->_yPos, this->_zPos, this->_size);
        } else if (this->_xPos == 0 && this->_yPos == 7 && this->_zPos == 0) {
            drawCube(cube, this->_xPos, this->_yPos + 1 - this->_size, this->_zPos, this->_size);
        } else if (this->_xPos == 0 && this->_yPos == 0 && this->_zPos == 7) {
            drawCube(cube, this->_xPos, this->_yPos, this->_zPos + 1 - this->_size, this->_size);
        } else if (this->_xPos == 7 && this->_yPos == 7 && this->_zPos == 0) {
            drawCube(cube, this->_xPos + 1 - this->_size, this->_yPos + 1 - this->_size, this->_zPos, this->_size);
        } else if (this->_xPos == 0 && this->_yPos == 7 && this->_zPos == 7) {
            drawCube(cube, this->_xPos, this->_yPos + 1 - this->_size, this->_zPos + 1 - this->_size, this->_size);
        } else if (this->_xPos == 7 && this->_yPos == 0 && this->_zPos == 7) {
            drawCube(cube, this->_xPos + 1 - this->_size, this->_yPos, this->_zPos + 1 - this->_size, this->_size);
        }
        if (this->_expanding) {
            if (this->_size++ == 8) {
                this->init(cube);
            }
        } else {
            if (this->_size-- == 1) {
                this->_expanding = true;
            }
        }
    }

   private:
    int _size = 2;
    bool _expanding = true;
    int _xPos;
    int _yPos;
    int _zPos;
};

// Glowing starts with an empty cube and turns on randomly all the LEDs one by one until fully lit,
// then turns then off one by one untill fully off again.
class Glowing : public BaseEffect {
   public:
    Glowing(unsigned int speed = 0) : BaseEffect(speed) {}

    // The _glowing flag controls whether we are currently lightning up the cube or turning it off.
    // When all LEDs are on or off, then switch the flag.
    // Each step, we pick a random LEDs not yet touched and flip it.
    void step(ICube& cube) {
        // If we are above the number of LEDs, flip, the glowing flag and reset the counter.
        if (this->_count >= 512) {
            this->_glowing = !this->_glowing;
            this->_count = 0;
            return;
        }

        int c = 0;
        int pick = ::random(0, 512 - this->_count++);

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                for (int k = 0; k < 8; k++) {
                    if (cube.getVoxel(i, j, k) == !this->_glowing) {
                        if (c++ == pick) {
                            cube.setVoxel(i, j, k, this->_glowing);
                            return;
                        }
                    }
                }
            }
        }
    }

   private:
    bool _glowing = true;
    int _count = 0;
};

// Drawing of the chars to display.
static const unsigned char _characters[][8] = {
    {
        0b00011000,
        0b00100100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00100100,
        0b00011000,
    },
    {
        0b00011000,
        0b00111000,
        0b11011000,
        0b10011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b01111110,
    },
    {
        0b11111111,
        0b00000001,
        0b00000001,
        0b00000001,
        0b11111111,
        0b10000000,
        0b10000000,
        0b11111111,
    },
    {
        0b11111111,
        0b00000001,
        0b00000001,
        0b00000001,
        0b11111111,
        0b00000001,
        0b00000001,
        0b11111111,
    },
    {
        0b10000001,
        0b10000001,
        0b10000001,
        0b10000001,
        0b11111111,
        0b00000001,
        0b00000001,
        0b00000001,
    },
    {
        0b11111111,
        0b10000000,
        0b10000000,
        0b10000000,
        0b11111111,
        0b00000001,
        0b00000001,
        0b11111111,
    },
    {
        0b11111111,
        0b11000000,
        0b11000000,
        0b11111111,
        0b11111111,
        0b11000011,
        0b11000011,
        0b11111111,
    },
    {
        0b11111111,
        0b11111111,
        0b00000110,
        0b00001100,
        0b00011000,
        0b00110000,
        0b01100000,
        0b11000000,
    },
    {
        0b11111111,
        0b11000011,
        0b11000011,
        0b11111111,
        0b11111111,
        0b11000011,
        0b11000011,
        0b11111111,
    },
    {
        0b11111111,
        0b11000011,
        0b11000011,
        0b11111111,
        0b11111111,
        0b00000011,
        0b00000011,
        0b00000011,
    },
};

// Numbers draw a number on a layer and shifts it accross the cube.
class Numbers : public BaseEffect {
   public:
    Numbers(unsigned int speed, Plane p = +Plane::Y) : BaseEffect(speed) {
        // If the requested plane is going in a negative direction, start from the 7, otherwise, start from 0.
        this->_plane = p(p == Plane::direction::negative ? 7 : 0);
    }

    void step(ICube& cube) {
        // If we are at the first/last layer (based on direction), draw the curent number.
        if (this->_plane == (this->_plane == Plane::direction::negative ? 7 : 0)) {
            // Just in case, make sure we have a clean state.
            cube.clear();
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    cube.setVoxel(this->_plane, i, 7 - j, _characters[this->_idx][j] & (1 << (7 - i)) ? 1 : 0);
                }
            }
        }
        // Shift the plane following the direction.
        ++this->_plane;
        cube.shift(this->_plane);
        // If we reach the other end, increment the number index and reset the plane to its original position.
        if (this->_plane == (this->_plane == Plane::direction::negative ? 0 : 7)) {
            this->_plane = this->_plane(this->_plane == Plane::direction::negative ? 7 : 0);
            this->_idx = (this->_idx + 1) % (sizeof(_characters) / sizeof(_characters[0]));
        }
    }

   private:
    Plane _plane;
    int _idx = 0;
};
