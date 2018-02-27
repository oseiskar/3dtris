#include "piece.hpp"

namespace pos_methods {
    Pos3d rotate(Pos3d pos, Rotation rot) {

        // 90 degeree rotation along X, Y or Z means exchanging/swapping
        // the values of the other two axes and flipping the sign on
        // one of these

        // which two axes are swapped by this rotation (order is important)
        int exAx0, exAx1;
        switch (rot.axis) {
        case Rotation::Axis::X:
            exAx0 = Rotation::Axis::Y;
            exAx1 = Rotation::Axis::Z;
            break;
        case Rotation::Axis::Y:
            exAx0 = Rotation::Axis::Z;
            exAx1 = Rotation::Axis::X;
            break;
        case Rotation::Axis::Z:
            exAx0 = Rotation::Axis::X;
            exAx1 = Rotation::Axis::Y;
            break;
        }

        // determines on which axis the sign is flipped
        const int sign = rot.direction == Rotation::Direction::CCW ? 1 : -1;

        // before & after
        const int p0[3] = { pos.x, pos.y, pos.z };
        int p1[3] = { pos.x, pos.y, pos.z };

        // swap & sign flip
        p1[exAx0] = -sign*p0[exAx1];
        p1[exAx1] = sign*p0[exAx0];

        return Pos3d { p1[0], p1[1], p1[2] };
    }
}

namespace block_methods {

    Block translate(const Block &target, Pos3d delta) {
        return Block {
            pos_methods::sum(target.pos, delta),
            target.material
        };
    }

    Block rotate(const Block &target, Rotation rot) {
        return Block {
            pos_methods::rotate(target.pos, rot),
            target.material
        };
    }
}

#include <algorithm>

// snippet from http://blog.madhukaraphatak.com/functional-programming-in-c++/
template <typename Collection,typename unop>
  Collection map(Collection col,unop op) {
  std::transform(col.begin(),col.end(),col.begin(),op);
  return col;
}

Piece Piece::translated(Pos3d delta) const {
    return Piece {
        pos_methods::sum(center, delta),
        blocks
    };
}

Piece Piece::rotated(Rotation rot) const {
    return Piece {
        center,
        map(blocks, [rot](Block b){ return block_methods::rotate(b, rot); })
    };
}

std::vector<Block> Piece::getBlocks() const {
    const Pos3d c = center;
    return map(blocks, [c](const Block &b){
        return block_methods::translate(b, c);
    });
}
