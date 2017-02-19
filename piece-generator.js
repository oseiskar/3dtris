"use strict";
/* globals Piece, Block */

function PieceGenerator() {

}

PieceGenerator.build = function() {
    const p = new PieceGenerator();
    return function() {
        return p.generate();
    };
};

PieceGenerator.prototype.generate = function() {
    return new Piece([
        new Block(0,0,0),
        new Block(1,0,0),
        new Block(0,1,0),
        new Block(0,0,1)
    ]);
};
