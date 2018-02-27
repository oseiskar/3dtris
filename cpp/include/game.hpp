#ifndef __GAME_HPP__
#define __GAME_HPP__
#include <vector>

struct Pos3d {
    int x, y, z;
};

struct Block {
    Pos3d pos;
    int material;
};

struct Rotation {
    enum Axis { X, Y, Z };
    enum Direction { CW, CCW };

    Axis axis;
    Direction direction;
};

class Game {
public:
    virtual std::vector<Block> getActiveBlocks() const;
    virtual std::vector<Block> getCementedBlocks() const;
    virtual bool isOver() const;
    virtual int getScore() const;

    // timed events
    virtual void tick(int dt);

    // controls
    virtual bool moveDown();
    virtual bool moveXY(int dx, int dy);
    virtual bool rotate(Rotation);
    virtual bool drop();
};

#endif
