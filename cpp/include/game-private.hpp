#ifndef __GAME_IMPLEMENTATION_HPP__
#define __GAME_IMPLEMENTATION_HPP__
#include "game.hpp"
#include "piece.hpp"

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

    //Piece activePiece;
    const Pos3d dims;
    //std::unordered_map<Pos3d, Block> cementedBlocks;

    bool isWithinBounds(Pos3d pos) const;
};

#endif
