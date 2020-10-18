#pragma once

#include "plane.h"

struct coords {
    coords(unsigned int _x, unsigned int _y, unsigned int _z) : x(_x), y(_y), z(_z) {}

    unsigned int& x;
    unsigned int& y;
    unsigned int& z;

    coords& operator=(const coords& in) {
        this->x = in.x;
        this->y = in.y;
        this->z = in.z;
        return *this;
    }
};

class ICubeRO {
   public:
    // Get a specific voxel.
    virtual int getVoxel(int x, int y, int z) const = 0;
    // Get the i,j voxel from the given plane.
    virtual int getVoxel(const Plane& p, int i, int j) const { return -1; }
};

class ICubeWO {
   public:
    // Set a specific voxel.
    virtual void setVoxel(int x, int y, int z, int value) = 0;
    // Set the i,j voxel from the given plane.
    virtual void setVoxel(const Plane& p, int i, int j, int value) {}

    // Get a mutable voxel.
    virtual int& getVoxelRef(const coords&) = 0;

    // Clear the full cube.
    virtual void clear(){};
    // Shift the full cube along the given plane.
    virtual void shift(const Plane& p){};
    // Fill a single plane layer with the given value.
    virtual void fill(const Plane& p, int value){};

    struct proxy {
        struct subproxy {
            subproxy(const proxy& p, int y = -1) : _p(p), _y(y) {}
            int& operator[](unsigned int idx) { return this->_p._cube.getVoxelRef({this->_p._x, this->_y, idx}); }
            const proxy& _p;
            unsigned int _y;
        };

        proxy(ICubeWO& cube, int x = -1) : _cube(cube), _x(x) {}
        subproxy operator[](unsigned int idx) { return subproxy(*this, idx); }
        ICubeWO& _cube;
        unsigned int _x;
    };

    // ex: cube[0][0][7] = 1;
    proxy operator[](unsigned int idx) { return proxy(*this, idx); }
    // ex: cube[{0, 0, 7}] = 1;
    int& operator[](const coords& c) { return this->getVoxelRef(c); }
};

class ICube : public ICubeRO, public ICubeWO {
};

class Cube : public ICube {
   public:
    Cube();

    void setVoxel(int x, int y, int z, int value);
    void setVoxel(const Plane& p, int i, int j, int value);

    int getVoxel(int x, int y, int z) const;
    int getVoxel(const Plane& p, int i, int j) const;
    int& getVoxelRef(const coords& c);

    void clear();

    void shift(const Plane& p);
    void fill(const Plane& p, int value);

   private:
    int _state[8][8][8];
};
