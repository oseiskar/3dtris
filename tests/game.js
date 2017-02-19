QUnit.test( "test game blockFitsBounds", function( assert ) {

    const game = new Game();

    assert.ok( game.blockFitsBounds(new Block(0,0,0)) );
    assert.ok( game.blockFitsBounds(new Block(game.dims.x-1,0,0)) );
    assert.ok( !game.blockFitsBounds(new Block(0,game.dims.y,0)) );
});

QUnit.test( "test game translateToBounds", function( assert ) {

    const game = new Game();
    const piece = new Piece([new Block(-2,0,0), new Block(0,-1,0)]);

    assert.ok(!game.pieceFits(piece));

    piece.translate(0,10,0);
    assert.ok(!game.pieceFits(piece));

    game.translateToBounds(piece);
    assert.ok(game.pieceFits(piece));

    const blocks = piece.getBlocks();
    assert.equal(blocks[0].x, 0);
    assert.equal(blocks[0].y, game.dims.y-1);
    assert.equal(blocks[1].x, 2);
    assert.equal(blocks[1].y, game.dims.y-2);
});


QUnit.test( "test game controls", function( assert ) {

    function pieceGenerator() {
        return new Piece([
            new Block(0,0,0),
            new Block(1,0,0),
            new Block(0,1,0),
            new Block(0,0,1)
        ]);
    }
    const game = new Game(pieceGenerator);

    let blocks = game.getActiveBlocks();
    assert.equal(blocks[0].x, Math.round(game.dims.x/2));
    assert.equal(blocks[1].x, Math.round(game.dims.x/2)+1);
    assert.equal(blocks[0].z, game.dims.z-2);
    assert.equal(blocks[3].z, game.dims.z-1);

    assert.ok(game.controls.moveDown());
    assert.ok(game.controls.moveXY(-1, 0));

    blocks = game.getActiveBlocks();
    assert.equal(blocks[0].x, Math.round(game.dims.x/2)-1);
    assert.equal(blocks[1].x, Math.round(game.dims.x/2));
    assert.equal(blocks[0].z, game.dims.z-3);
    assert.equal(blocks[3].z, game.dims.z-2);

    assert.ok(game.controls.drop());
});
