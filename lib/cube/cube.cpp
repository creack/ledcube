#include "cube.h"

Cube::Cube() {
    this->clear();
}

void Cube::setVoxel(int x, int y, int z, int value) {
    this->_state[x][y][z] = value ? 1 : 0;
}

void Cube::setVoxel(const Plane& p, int i, int j, int value) {
    Plane::axis axis = p;
    int offset = p;

    this->setVoxel(
        axis == Plane::axis::x ? offset : i,
        axis == Plane::axis::y ? offset : j,
        axis == Plane::axis::z ? offset : axis == Plane::axis::x ? i : j,
        value);
}

int Cube::getVoxel(int x, int y, int z) const {
    return this->_state[x][y][z];
}

int& Cube::getVoxelRef(const coords& c) {
    return this->_state[c.x][c.y][c.z];
}

int Cube::getVoxel(const Plane& p, int i, int j) const {
    Plane::axis axis = p;
    int offset = p;

    return this->getVoxel(
        axis == Plane::axis::x ? offset : i,
        axis == Plane::axis::y ? offset : j,
        axis == Plane::axis::z ? offset : axis == Plane::axis::x ? i : j);
}

void Cube::fill(const Plane& p, int value) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            this->setVoxel(p, i, j, value);
        }
    }
}

void Cube::clear() {
    for (int z = 0; z < 8; z++) {
        this->fill(Plane::Z(z), 0);
    }
}

void Cube::shift(const Plane& p) {
    if (p == Plane::direction::stale) {
        return;
    }

    const Plane::direction direction = p;

    for (int k = direction < 0 ? 0 : 7; (direction < 0 && k < 7) || (direction > 0 && k > 0); k -= direction) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                this->setVoxel(p(k), i, j, this->getVoxel(p(k - direction), i, j));
            }
        }
    }

    // Clear out the first/last layer.
    this->fill(p(direction < 0 ? 7 : 0), 0);
}
