#include "piece.hpp"
#include <array>
#include <algorithm>
#include <iterator>
#include <assert.h>

namespace pos_methods {

    Pos3d fromArray(std::array<int, 3> arr) {
        return Pos3d { arr[0], arr[1], arr[2] };
    }

    std::array<int, 3> toArray(const Pos3d& pos) {
        return {{ pos.x, pos.y, pos.z }};
    }

    Pos3d rotate(Pos3d pos, Rotation rot) {

        // 90 degeree rotation along X, Y or Z means exchanging/swapping
        // the values of the other two axes and flipping the sign on
        // one of these

        // which two axes are swapped by this rotation (order is important)
        Axis exAx0, exAx1;
        switch (rot.axis) {
        case Axis::X:
            exAx0 = Axis::Y;
            exAx1 = Axis::Z;
            break;
        case Axis::Y:
            exAx0 = Axis::Z;
            exAx1 = Axis::X;
            break;
        case Axis::Z:
            exAx0 = Axis::X;
            exAx1 = Axis::Y;
            break;
        }

        // determines on which axis the sign is flipped
        const int sign = rot.direction == RotationDirection::CCW ? 1 : -1;

        // before & after
        auto p0 = toArray(pos);
        decltype(p0) p1 = p0;

        // swap & sign flip
        p1[static_cast<int>(exAx0)] = -sign*p0[static_cast<int>(exAx1)];
        p1[static_cast<int>(exAx1)] = sign*p0[static_cast<int>(exAx0)];

        return fromArray(p1);
    }

    Pos3d setElement(const Pos3d &pos, Axis ax, int value) {
        auto a = toArray(pos);
        a[static_cast<int>(ax)] = value;
        return fromArray(a);
    }

    int getElement(const Pos3d &pos, Axis ax) {
        return toArray(pos)[static_cast<int>(ax)];
    }
}

namespace block_methods {

    Block translate(const Block &target, Pos3d delta) {
        return Block {
            pos_methods::sum(target.pos, delta),
            target.pieceId
        };
    }

    Block rotate(const Block &target, Rotation rot) {
        return Block {
            pos_methods::rotate(target.pos, rot),
            target.pieceId
        };
    }
}

// snippet from http://blog.madhukaraphatak.com/functional-programming-in-c++/
template <typename Collection,typename unop>
Collection map(Collection col, unop op) {
    std::transform(col.begin(), col.end(), col.begin(), op);
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

int Piece::getExtent(Axis axis, int direction) const {
    assert(direction == 1 || direction == -1);

    auto blocks = getBlocks();

    std::vector<int> coords;
    for (auto b : blocks) {
        coords.push_back(pos_methods::getElement(b.pos, axis));
    }

    return direction > 0 ?
        *std::max_element(coords.begin(), coords.end()) :
        *std::min_element(coords.begin(), coords.end());
}

Piece Piece::translatedBeyond(Axis axis, int limit, int direction) const {
    const int extreme = getExtent(axis, -direction);
    const int diff = (limit - extreme)*direction;

    if (diff > 0) {
        return translated(
            pos_methods::setElement(
                Pos3d { 0, 0, 0 },
                axis,
                diff*direction)
        );
    }
    return *this;
}

std::vector<Block> Piece::getBlocks() const {
    const Pos3d c = center;
    return map(blocks, [c](const Block &b){
        return block_methods::translate(b, c);
    });
}
