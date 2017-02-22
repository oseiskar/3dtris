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

PieceGenerator.prototype.blocks = [
    [
        new Block(-1,0,0), //  ##
        new Block(0,0,0),  // ##
        new Block(0,1,0),
        new Block(1,1,0)
    ],
    [
        new Block(-1,0,0), // ####
        new Block(0,0,0),
        new Block(1,0,0),
        new Block(2,0,0)
    ],
    [
        new Block(-1,0,0), // ###
        new Block(0,0,0),  //   #
        new Block(1,0,0),
        new Block(1,1,0)
    ],
    [
        new Block(-1,0,0), // ###
        new Block(0,0,0),  //  #
        new Block(1,0,0),
        new Block(0,1,0)
    ],
    [
        new Block(0,0,0), // ##
        new Block(0,1,0),  // ##
        new Block(1,0,0),
        new Block(1,1,0)
    ],
    [
        new Block(-1,0,0), // ##
        new Block(0,0,0),  //  o
        new Block(0,1,1),
        new Block(0,1,0)
    ],
    [
        new Block(-1,0,0), // o#
        new Block(0,0,0),  //  #
        new Block(-1,0,1),
        new Block(0,1,0)
    ],
    [
        new Block(0,0,0), // o#
        new Block(1,0,0), // #
        new Block(0,1,0),
        new Block(0,0,1)
    ],
];

PieceGenerator.prototype.generate = function() {
    const blocks = PieceGenerator.prototype.blocks;
    return new Piece(blocks[Math.floor(Math.random()*blocks.length)]);
};
