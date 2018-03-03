#include "cemented-block-array.hpp"
#include <assert.h>

CementedBlockArray::CementedBlockArray(const GameBox& gameBox)
:
    box(gameBox),
    nonEmpty(gameBox.size()),
    blockPieceIds(gameBox.size())
{}

int CementedBlockArray::posToIndex(Pos3d pos) const {
    return pos.z*box.dims.x*box.dims.y + pos.y*box.dims.x + pos.x;
}

void CementedBlockArray::clearBlock(Pos3d pos) {
    nonEmpty[posToIndex(pos)] = false;
}

int CementedBlockArray::getBlockPieceId(Pos3d pos) const {
    assert( hasBlock(pos) );
    return blockPieceIds[posToIndex(pos)];
}

void CementedBlockArray::setBlock(const Block &block) {
    assert( box.contains(block.pos) );

    int idx = posToIndex(block.pos);
    nonEmpty[idx] = true;
    blockPieceIds[idx] = block.pieceId;
}

bool CementedBlockArray::hasBlock(Pos3d pos) const {
    assert( box.contains(pos) );
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

void CementedBlockArray::removeLayer(int zToRemove) {
    for (int z = zToRemove; z < box.dims.z; ++z) {
        for (int x = 0; x < box.dims.x; ++x) {
            for (int y = 0; y < box.dims.y; ++y) {
                const Pos3d pos {x,y,z}, top {x,y,z+1};
                if (z == box.dims.z-1 || !hasBlock(top)) clearBlock(pos);
                else setBlock(Block{pos, getBlockPieceId(top)});
            }
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
                    blocks.push_back(Block{pos, getBlockPieceId(pos)});
            }
        }
    }
    return blocks;
}

bool CementedBlockArray::pieceFits(const Piece& piece) const {
    for (Block b : piece.getBlocks()) {
        if (!box.contains(b.pos) || hasBlock(b.pos)) return false;
    }
    return true;
}

void CementedBlockArray::cementPiece(const Piece& piece) {
    assert( pieceFits(piece) );
    for (Block b : piece.getBlocks()) setBlock(b);
}
