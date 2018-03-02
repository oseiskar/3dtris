#include "game-box.hpp"

bool GameBox::contains(Pos3d p) const {
    return p.x >= 0 && p.x < dims.x &&
        p.y >= 0 && p.y < dims.y &&
        p.z >= 0 && p.z < dims.z;
}

bool GameBox::contains(const Piece& p) const {
    for (Block b : p.getBlocks()) {
        if (!contains(b.pos)) return false;
    }
    return true;
}

Piece GameBox::translateToBounds(const Piece& p) const {
    Piece piece = p;
    for (Axis axis : { Axis::X, Axis::Y, Axis::Z }) {
        const int min = piece.getExtent(axis, -1);
        if (min < 0)
            piece = piece.translatedBeyond(axis, 0, 1);

        const int max = piece.getExtent(axis, 1);
        const int limit = pos_methods::getElement(dims, axis);
        if (max >= limit)
            piece = piece.translatedBeyond(axis, limit-1, -1);
    }
    return piece;
}
