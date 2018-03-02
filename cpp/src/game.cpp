#include "game.hpp"

std::unique_ptr<Game> buildGame() {
    return std::unique_ptr<Game>(new ConcreteGame());
}

ConcreteGame::ConcreteGame()
:
    gameBox(Pos3d { 5, 6, 15 }),
    blockArray(gameBox),
    activePiece {{}}, // empty piece
    score(0),
    alive(true)
{}

std::vector<Block> ConcreteGame::getActiveBlocks() const {
    return activePiece.getBlocks();
}

std::vector<Block> ConcreteGame::getCementedBlocks() const {
    return blockArray.getNonEmptyBlocks();
}

bool ConcreteGame::isOver() const {
    return !alive;
}

int ConcreteGame::getScore() const {
    return score;
}

// timed events
void ConcreteGame::tick(int dt) {
    abort();
}

// controls

bool ConcreteGame::moveXY(int dx, int dy) {
    return setActivePieceIfFits(activePiece.translated(Pos3d {dx,dy,0}));
}

bool ConcreteGame::rotate(Rotation rot) {
    return setActivePieceIfFits(
        gameBox.translateToBounds(activePiece.rotated(rot)));
}

void ConcreteGame::drop() {
    // TODO: score
    while (moveDown());
}

// private helpers
bool ConcreteGame::moveDown() {
    if (!setActivePieceIfFits(activePiece.translated(Pos3d {0,0,-1}))) {
        blockArray.cementPiece(activePiece);
        // TODO: new piece
        return false;
    }
    return true;
}

bool ConcreteGame::setActivePieceIfFits(const Piece& candidate) {
    if (!blockArray.pieceFits(candidate)) {
        return false;
    }
    activePiece = candidate;
    return true;
}
