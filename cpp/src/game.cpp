#include "game-private.hpp"

std::unique_ptr<Game> buildGame() {
    return std::unique_ptr<Game>(new ConcreteGame());
}

ConcreteGame::ConcreteGame()
: dims { 5, 6, 15 }
{
}

std::vector<Block> ConcreteGame::getActiveBlocks() const {
    abort();
    return std::vector<Block>();
    //return activePiece.getBlocks();
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
    abort();
    return 0;
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

bool ConcreteGame::isWithinBounds(Pos3d p) const {
    return p.x >= 0 && p.x < dims.x &&
        p.y >= 0 && p.y < dims.y &&
        p.z >= 0 && p.z < dims.z;
}
