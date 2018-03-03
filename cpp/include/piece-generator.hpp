#ifndef __PIECE_GENERATOR_HPP__
#define __PIECE_GENERATOR_HPP__

#include "piece.hpp"
#include "game-box.hpp"
#include <vector>
#include <random>

class PieceGenerator {
private:
    std::mt19937 random;
    const GameBox& gameBox;
    std::vector< std::vector<Pos3d> > prototypes;

    int nextMaterial();
    Piece randomTransformation(const Piece& original);
public:
    PieceGenerator(const GameBox &gameBox, int randomSeed);
    Piece nextPiece();
};

#endif
