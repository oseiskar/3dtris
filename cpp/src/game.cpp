#include "game.hpp"

std::unique_ptr<Game> buildGame() {
    return std::unique_ptr<Game>(new ConcreteGame());
}

ConcreteGame::ConcreteGame()
:
    gameBox(Pos3d { 5, 6, 15 }),
    activePiece {{}}, // empty piece
    score(0)
{}

std::vector<Block> ConcreteGame::getActiveBlocks() const {
    return activePiece.getBlocks();
}

std::vector<Block> ConcreteGame::getCementedBlocks() const {
    abort();
    return std::vector<Block>();
}

bool ConcreteGame::isOver() const {
    abort();
    return false;
}

int ConcreteGame::getScore() const {
    return score;
}

// timed events
void ConcreteGame::tick(int dt) {
    abort();
}

// controls
bool ConcreteGame::moveDown() {
    abort();
    return false;
}

bool ConcreteGame::moveXY(int dx, int dy) {
    abort();
    return false;
}

bool ConcreteGame::rotate(Rotation) {
    abort();
    return false;
}

bool ConcreteGame::drop() {
    abort();
    return false;
}

// --- helpers

bool GameBox::contains(Pos3d p) const {
    return p.x >= 0 && p.x < dims.x &&
        p.y >= 0 && p.y < dims.y &&
        p.z >= 0 && p.z < dims.z;
}

bool GameBox::contains(const Piece& p) const {
    for (Block b : p.getBlocks()) {
        if (!contains(b.pos)) return false;
    }
    return true;
}

Piece GameBox::translateToBounds(const Piece& p) const {
    Piece piece = p;
    for (Axis axis : { Axis::X, Axis::Y, Axis::Z }) {
        const int min = piece.getExtent(axis, -1);
        if (min < 0)
            piece = piece.translatedBeyond(axis, 0, 1);

        const int max = piece.getExtent(axis, 1);
        const int limit = pos_methods::getElement(dims, axis);
        if (max >= limit)
            piece = piece.translatedBeyond(axis, limit-1, -1);
    }
    return piece;
}

// --- cemented blocks

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
