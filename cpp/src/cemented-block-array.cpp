#include "cemented-block-array.hpp"

CementedBlockArray::CementedBlockArray(const GameBox& gameBox)
:
    box(gameBox),
    nonEmpty(gameBox.size()),
    blockMaterials(gameBox.size())
{}

int CementedBlockArray::posToIndex(Pos3d pos) const {
    return pos.z*box.dims.x*box.dims.y + pos.y*box.dims.x + pos.x;
}

void CementedBlockArray::clearBlock(Pos3d pos) {
    nonEmpty[posToIndex(pos)] = false;
}

int CementedBlockArray::getBlockMaterial(Pos3d pos) const {
    if (!hasBlock(pos)) abort();
    return blockMaterials[posToIndex(pos)];
}

void CementedBlockArray::setBlock(const Block &block) {
    if (!box.contains(block.pos)) abort();

    int idx = posToIndex(block.pos);
    nonEmpty[idx] = true;
    blockMaterials[idx] = block.material;
}

bool CementedBlockArray::hasBlock(Pos3d pos) const {
    if (!box.contains(pos)) abort();
    return nonEmpty[posToIndex(pos)];
}

bool CementedBlockArray::isLayerFull(int z) const {
    for (int x = 0; x < box.dims.x; ++x) {
        for (int y = 0; y < box.dims.y; ++y) {
            if (!hasBlock(Pos3d {x,y,z})) return false;
        }
    }
    return true;
}

void CementedBlockArray::removeLayer(int z) {
    for (int x = 0; x < box.dims.x; ++x) {
        for (int y = 0; y < box.dims.y; ++y) {
            const Pos3d pos {x,y,z}, top {x,y,z+1};
            if (z == box.dims.z-1 || !hasBlock(top)) clearBlock(pos);
            else setBlock(Block{pos, getBlockMaterial(top)});
        }
    }
}

std::vector<Block> CementedBlockArray::getNonEmptyBlocks() const {
    std::vector<Block>  blocks;
    for (int z = 0; z < box.dims.z; ++z) {
        for (int y = 0; y < box.dims.y; ++y) {
            for (int x = 0; x < box.dims.x; ++x) {
                const Pos3d pos {x,y,z};
                if (hasBlock(pos))
                    blocks.push_back(Block{pos, getBlockMaterial(pos)});
            }
        }
    }
    return blocks;
}
