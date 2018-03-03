#ifndef __GAME_IMPLEMENTATION_HPP__
#define __GAME_IMPLEMENTATION_HPP__
#include "api.hpp"
#include "piece.hpp"
#include "game-box.hpp"
#include "cemented-block-array.hpp"
#include "piece-generator.hpp"
#include <bitset>

class ConcreteGame : public Game {
private:
    bool rotate(Rotation);
public:
    ConcreteGame(unsigned int randomSeed);

    std::vector<Block> getActiveBlocks() const override;
    std::vector<Block> getCementedBlocks() const override;

    bool isOver() const override;
    int getScore() const override;
    Pos3d getDimensions() const override;

    // timed events
    bool tick(int dtMilliseconds) override;

    // controls
    bool moveXY(int dx, int dy) override;
    void drop() override;
    bool rotate(Axis axis, RotationDirection dir) override;

    virtual ~ConcreteGame() = default;

private:
    bool moveDown();

    // set active piece to given candidate and return true. If it does not fit,
    // do not change active piece and return false.
    bool setActivePieceIfFits(const Piece& candidate);

    const GameBox gameBox;
    CementedBlockArray blockArray;
    PieceGenerator pieceGenerator;
    Piece activePiece;

    int score;
    bool alive;

    int timeToNextDownMs;
    int nDroppedPieces;
};

#endif
