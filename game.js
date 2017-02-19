"use strict";
/* globals PieceGenerator */

function Game(pieceGenerator) {

    if (!pieceGenerator) {
        pieceGenerator = PieceGenerator.build();
    }

    this.dims = {
        x: 5,
        y: 6,
        z: 15
    };
    const that = this;

    const blocks = [];

    for (var i=0; i<this.dims.x*this.dims.y*this.dims.z; ++i)
        blocks.push(null);

    var activePiece;

    function newPiece() {
        activePiece = pieceGenerator();
        activePiece.translateBeyond('z', that.dims.z-1, 1);
        activePiece.translate(
            Math.round(that.dims.x/2),
            Math.round(that.dims.y/2),
            0);
        that.translateToBounds(activePiece);
        return that.pieceFits(activePiece);
    }

    this.getActiveBlocks = function() {
        return activePiece.getBlocks();
    };

    function blockIndex(xyz) {
        return xyz.z*that.dims.x*that.dims.y + xyz.y*that.dims.x + xyz.x;
    }

    this.blockFits = function(b) {
        return that.blockFitsBounds(b) && blocks[blockIndex(b)] === null;
    };

    function tryMove(func) {
        const testPiece = activePiece.copy();
        func(testPiece);
        if (that.pieceFits(testPiece)) {
            func(activePiece);
            return true;
        }
        return false;
    }

    function cementPiece() {
        activePiece.getBlocks().forEach(b => {
            blocks[blockIndex[b]] = b;
        });
        return newPiece();
    }

    this.controls = {
        moveDown: function() {
            if (!tryMove(p => p.translate(0,0,-1))) {
                cementPiece();
                return false;
            }
            return true;
        },
        moveXY: function(dx, dy) {
            return tryMove(p => p.translate(dx,dy,0));
        },
        rotate: function(axis, dir) {
            return tryMove(p => {
                p.rotate(axis, dir);
                that.translateToBounds(p);
            });
        }
    };

    this.controls.drop = function() {
        while (that.controls.moveDown());
        return true;
    };

    newPiece();
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
