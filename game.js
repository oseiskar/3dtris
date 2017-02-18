"use strict";

function Piece(blocks) {
    this.blocks = blocks;
}

Piece.prototype.rotate = function(axis, sign) {
    this.blocks.foreach(b => {
        b.assign(rotate(b, axis, sign));
    });
};

function Block(x, y, z) {
    this.x = x;
    this.y = y;
    this.z = z;
}

Block.prototype.assign = function(other) {
    this.x = other.x;
    this.y = other.y;
    this.z = other.z;
};

function rotate(block, axis, sign) {
    let exchangedAxes = {
        x: ['y', 'z'],
        y: ['z', 'x'],
        z: ['x', 'y']
    }[axis];

    if (!exchangedAxes) {
        throw "invalid axis "+axis;
    }

    let b = new Block(block.x, block.y, block.z);
    b[exchangedAxes[0]] = -sign*block[exchangedAxes[1]];
    b[exchangedAxes[1]] = sign*block[exchangedAxes[0]];
    return b;
}
