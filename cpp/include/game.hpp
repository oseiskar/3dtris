#ifndef __GAME_IMPLEMENTATION_HPP__
#define __GAME_IMPLEMENTATION_HPP__
#include "api.hpp"
#include "piece.hpp"
#include "game-box.hpp"
#include "cemented-block-array.hpp"
#include <bitset>

class ConcreteGame : public Game {
public:
    ConcreteGame();

    std::vector<Block> getActiveBlocks() const override;
    std::vector<Block> getCementedBlocks() const override;

    bool isOver() const override;
    int getScore() const override;

    // timed events
    void tick(int dt) override;

    // controls
    bool moveXY(int dx, int dy) override;
    bool rotate(Rotation) override;
    void drop() override;

    virtual ~ConcreteGame() = default;

private:
    bool moveDown();

    // set active piece to given candidate and return true. If it does not fit,
    // do not change active piece and return false.
    bool setActivePieceIfFits(const Piece& candidate);

    const GameBox gameBox;
    CementedBlockArray blockArray;
    Piece activePiece;

    int score;
    bool alive;
};

#endif
