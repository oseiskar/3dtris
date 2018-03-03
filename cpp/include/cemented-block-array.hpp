#ifndef __CEMENTED_BLOCK_ARRAY_HPP__
#define __CEMENTED_BLOCK_ARRAY_HPP__

#include "game-box.hpp"

class CementedBlockArray {
private:
    const GameBox& box;
    std::vector<bool> nonEmpty;
    std::vector<int> blockPieceIds;

    void clearBlock(Pos3d pos);
    int getBlock(Pos3d pos) const;
    int getBlockPieceId(Pos3d pos) const;

    int posToIndex(Pos3d pos) const;
public:
    CementedBlockArray(const GameBox& gameBox);

    bool pieceFits(const Piece& piece) const;
    void cementPiece(const Piece& piece);

    bool isLayerFull(int z) const;
    void removeLayer(int z);
    std::vector<Block> getNonEmptyBlocks() const;

    // helpers
    void setBlock(const Block& block);
    bool hasBlock(Pos3d pos) const;
};

#endif
