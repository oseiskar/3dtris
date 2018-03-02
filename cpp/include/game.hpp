#ifndef __GAME_IMPLEMENTATION_HPP__
#define __GAME_IMPLEMENTATION_HPP__
#include "api.hpp"
#include "piece.hpp"
#include <bitset>

class GameBox {
public:
    GameBox(Pos3d d) : dims(d) {}
    const Pos3d dims;

    bool contains(Pos3d pos) const;
    bool contains(const Piece& piece) const;
    Piece translateToBounds(const Piece &piece) const;
    int size() const { return dims.x*dims.y*dims.z; }
};

class CementedBlockArray {
private:
    const GameBox& box;
    std::vector<bool> nonEmpty;
    std::vector<int> blockMaterials;

    void clearBlock(Pos3d pos);
    int getBlock(Pos3d pos) const;
    int getBlockMaterial(Pos3d pos) const;

    int posToIndex(Pos3d pos) const;
public:
    CementedBlockArray(const GameBox& gameBox);

    void setBlock(const Block& block);
    bool hasBlock(Pos3d pos) const;
    bool isLayerFull(int z) const;
    void removeLayer(int z);
    std::vector<Block> getNonEmptyBlocks() const;
};

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
