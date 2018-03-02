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
    bool moveDown() override;
    bool moveXY(int dx, int dy) override;
    bool rotate(Rotation) override;
    bool drop() override;

    virtual ~ConcreteGame() = default;

private:
    const GameBox gameBox;
    Piece activePiece;
    //Piece activePiece;
    //std::unordered_map<Pos3d, Block> cementedBlocks;

    int score;
};

#endif
