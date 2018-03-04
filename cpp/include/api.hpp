#ifndef __GAME_HPP__
#define __GAME_HPP__
#include <vector>
#include <memory>

struct Pos3d {
    int x, y, z;
};

struct Block {
    Pos3d pos;
    int pieceId;
};

enum class Axis {
    X, Y, Z
};

enum class RotationDirection {
    CW, CCW
};

class Game {
public:
    virtual std::vector<Block> getActiveBlocks() const = 0;
    virtual std::vector<Block> getCementedBlocks() const = 0;
    virtual std::vector<Block> getAllBlocks() const = 0;

    virtual bool isOver() const = 0;
    virtual int getScore() const = 0;
    virtual Pos3d getDimensions() const = 0;

    // timed events
    virtual bool tick(int dtMilliseconds) = 0;

    // controls
    virtual bool moveXY(int dx, int dy) = 0;
    virtual void drop() = 0;

    virtual bool rotate(Axis axis, RotationDirection dir) = 0;

    virtual ~Game() = default;
};

std::unique_ptr<Game> buildGame(unsigned int randomSeed);

#endif
