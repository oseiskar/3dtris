"use strict";

function Block(x, y, z) {
    this.x = x;
    this.y = y;
    this.z = z;
}

Block.prototype.assign = function(other) {
    this.x = other.x;
    this.y = other.y;
    this.z = other.z;
    return this;
};

Block.prototype.rotated = function(axis, sign) {

    let exchangedAxes = {
        x: ['y', 'z'],
        y: ['z', 'x'],
        z: ['x', 'y']
    }[axis];

    if (!exchangedAxes) {
        throw "invalid axis "+axis;
    }

    let b = new Block(0,0,0).assign(this);
    b[exchangedAxes[0]] = -sign*this[exchangedAxes[1]];
    b[exchangedAxes[1]] = sign*this[exchangedAxes[0]];
    return b;
};

Block.prototype.translated = function(dx, dy, dz) {
    return new Block(this.x + dx, this.y + dy, this.z + dz);
};

function Piece(blocks_) {
    var center = {
        x: 0,
        y: 0,
        z: 0
    };

    var blocks = blocks_.map(b => new Block(0,0,0).assign(b));

    this.rotate = function(axis, sign) {
        blocks.forEach(b => {
            b.assign(b.rotated(axis, sign));
        });
    };

    this.translateDict = function(d) {
         ['x', 'y', 'z'].forEach(k => {
            center[k] += d[k];
        });
    };

    this.translate = function(dx, dy, dz) {
        this.translateDict({x: dx, y: dy, z: dz});
    };


    this.getBlocks = function() {
        return blocks.map(b => b.translated(center.x, center.y, center.z));
    };
}

Piece.prototype.translateBeyond = function(axis, limit, dir) {
    let func;
    if (dir == 1) {
        func = (a,b) => Math.min(a,b);
    } else if (dir == -1) {
        func = (a,b) => Math.max(a,b);
    } else {
        throw "invalid direction "+dir;
    }

    const extreme = this.getBlocks().map(b => b[axis]).reduce(func);

    let diff = (limit - extreme)*dir;

    if (diff > 0) {
        let trans = {x: 0, y: 0, z: 0};
        trans[axis] = diff*dir;
        this.translateDict(trans);
        return true;
    }

    return false;
};
