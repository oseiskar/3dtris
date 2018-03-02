#ifndef __GAME_BOX_HPP__
#define __GAME_BOX_HPP__

#include "piece.hpp"

class GameBox {
public:
    GameBox(Pos3d d) : dims(d) {}
    const Pos3d dims;

    bool contains(Pos3d pos) const;
    bool contains(const Piece& piece) const;
    Piece translateToBounds(const Piece &piece) const;
    int size() const { return dims.x*dims.y*dims.z; }
};

#endif
