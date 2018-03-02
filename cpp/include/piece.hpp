#ifndef __PIECE_HPP__
#define __PIECE_HPP__

#include "api.hpp"
#include <vector>

struct Piece {
private:
    Pos3d center;
    std::vector<Block> blocks;

public:
    Piece(std::vector<Block> blocks_)
    : Piece(Pos3d {0,0,0}, blocks_) {}

    Piece(Pos3d center_, std::vector<Block> blocks_)
    : center(center_), blocks(blocks_) {}

    Piece(const Piece& other) = default;
    std::vector<Block> getBlocks() const;

    Piece translated(Pos3d) const;
    Piece rotated(Rotation) const;

    // would not need access to private data
    Piece translatedBeyond(Axis axis, int limit, int direction) const;
    int getExtent(Axis axis, int direction) const;
};

namespace block_methods {
    Block rotate(const Block&, Rotation);
    Block translate(const Block&, Pos3d);
}

namespace pos_methods {
    inline Pos3d sum(Pos3d a, Pos3d b) {
        return Pos3d { a.x + b.x, a.y + b.y, a.z + b.z };
    }

    int getElement(const Pos3d &pos, Axis ax);
}

#endif
