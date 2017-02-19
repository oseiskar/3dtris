"use strict";

function Game() {
    const dims = {
        x: 5,
        y: 6,
        z: 15
    };
    this.dims = dims;
    const that = this;

    const blocks = [];
    for (var i=0; i<this.dims.x*this.dims.y*this.dims.z; ++i)
        blocks.push(null);

    function blockIndex(xyz) {
        return xyz.z*dims.x*dims.y + xyz.y*dims.x + xyz.x;
    }

    this.blockFits = function(b) {
        return that.blockFitsBounds(b) && blocks[blockIndex(b)] === null;
    };
}

Game.prototype.blockFitsBounds = function(b) {
    const d = this.dims;
    return ['x', 'y', 'z'].every(c => b[c] >= 0 && b[c] < d[c]);
};

Game.prototype.translateToBounds = function(piece) {
    const dims = this.dims;
    ['x', 'y', 'z'].forEach(c => {
        piece.translateBeyond(c, 0, 1);
        piece.translateBeyond(c, dims[c]-1, -1);
    });
};

Game.prototype.pieceFits = function(piece) {
    const that = this;
    return piece.getBlocks().every(b => that.blockFits(b));
};
