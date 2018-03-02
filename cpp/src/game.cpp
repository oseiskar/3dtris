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
