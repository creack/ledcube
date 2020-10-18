#pragma once

#include "cube.h"

// IBoard defines the base methods for a board to be used.
class IBoard {
    typedef int (*voxelMapping)(const ICubeRO& cube, int x, int y, int z);

   public:
    virtual void setup() const = 0;
    virtual void render(const ICubeRO&) const = 0;

    virtual void setMapping(voxelMapping mapping) {
        this->_mapping = mapping;
    }

    virtual int getVoxel(const ICubeRO& cube, int x, int y, int z) const {
        if (this->_mapping != 0) {
            return this->_mapping(cube, x, y, z);
        }
        return cube.getVoxel(x, y, z);
    }

   protected:
    voxelMapping _mapping = 0;
};
