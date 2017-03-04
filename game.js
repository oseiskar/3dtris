"use strict";
/* globals PieceGenerator */

function Game(pieceGenerator) {

    if (!pieceGenerator) {
        pieceGenerator = PieceGenerator.build();
    }

    this.dims = {
        x: 5,
        y: 5,
        z: 14
    };

    this.score = 0;

    const that = this;

    const blocks = [];

    for (var i=0; i<this.dims.x*this.dims.y*this.dims.z; ++i)
        blocks.push(null);

    var activePiece = null;

    function newPiece() {
        activePiece = pieceGenerator();
        that.score += 1;

        function randomRot(axis) {
            const limit = Math.random()*4;
            tryMove(p => {
                for (var i=0; i<limit; ++i) {
                    p.rotate(axis, 1);
                }
                that.translateToBounds(p);
            });
        }

        // too lazy to generate the rotation group correctly
        randomRot('x');
        randomRot('y');
        randomRot('z');

        activePiece.translateBeyond('z', that.dims.z-1, 1);
        activePiece.translate(
            Math.floor(Math.random()*that.dims.x),
            Math.floor(Math.random()*that.dims.y),
            0);
        that.translateToBounds(activePiece);
        if (!that.pieceFits(activePiece)) {
            activePiece = null;
        }
    }

    this.isOver = function() {
        return activePiece === null;
    };

    this.getActiveBlocks = function() {
        if (activePiece) {
            return activePiece.getBlocks();
        } else {
            return [];
        }
    };

    this.getCementedBlocks = function() {
        var r = [];
        for (let z=0; z<that.dims.z; ++z) {
            for (let x=0; x<that.dims.x; ++x) {
                for (let y=0; y<that.dims.y; ++y) {
                    const b = blocks[blockIndex({x:x,y:y,z:z})];
                    if (b) r.push(b);
                }
            }
        }
        return r;
    };

    function blockIndex(xyz) {
        return xyz.z*that.dims.x*that.dims.y + xyz.y*that.dims.x + xyz.x;
    }

    function isBlockEmpty(b) {
         return blocks[blockIndex(b)] === null;
    }

    this.blockFits = function(b) {
        return that.blockFitsBounds(b) && isBlockEmpty(b);
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

    function isLayerFull(z) {
        for (let x=0; x<that.dims.x; ++x) {
            for (let y=0; y<that.dims.y; ++y) {
                if (isBlockEmpty({x:x,y:y,z:z})) {
                    return false;
                }
            }
        }
        return true;
    }

    function removeLayer(z) {
        that.score += 100;
        for (; z<that.dims.z; ++z) {
            for (let x=0; x<that.dims.x; ++x) {
                for (let y=0; y<that.dims.y; ++y) {
                    const trgIdx = blockIndex({x:x,y:y,z:z});
                    if (z === that.dims.z - 1) {
                        blocks[trgIdx] = null;
                    } else {
                        const srcIdx = blockIndex({x:x,y:y,z:z+1});
                        blocks[trgIdx] = blocks[srcIdx];
                        if (blocks[trgIdx]) {
                            blocks[trgIdx].z--;
                        }
                    }
                }
            }
        }
    }

    function tryRemoveLayers() {
        for (let z=0; z<that.dims.z; ++z) {
            while (isLayerFull(z)) {
                removeLayer(z);
            }
        }
    }

    function cementPiece() {
        activePiece.getBlocks().forEach(b => {
            blocks[blockIndex(b)] = b;
        });
        tryRemoveLayers();
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
