#ifndef __CEMENTED_BLOCK_ARRAY_HPP__
#define __CEMENTED_BLOCK_ARRAY_HPP__

#include "game-box.hpp"

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

#endif
