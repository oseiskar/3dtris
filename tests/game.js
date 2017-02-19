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

    assert.ok(!game.isOver());

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

    assert.ok(game.controls.rotate('y', -1));
    assert.ok(game.controls.rotate('y', 1));
    assert.ok(game.controls.drop());

    let cem = game.getCementedBlocks();
    assert.equal(cem.length, 4);
    assert.equal(cem[0].x, Math.round(game.dims.x/2)-1);
    assert.equal(cem[0].y, Math.round(game.dims.y/2));
    assert.equal(cem[0].z, 0);
    assert.equal(cem[3].z, 1);

    assert.ok(!game.isOver());

    while(!game.isOver()) {
        game.controls.drop();
    }

    assert.ok(game.isOver());
});

QUnit.test( "test game remove bottom", function( assert ) {

    const dims = new Game().dims;

    function pieceGenerator() {
        // two full layers + one block on top
        let blocks = [new Block(1,1,2)];
        for (let z=0; z<2; ++z) {
            for (let x=0; x<dims.x; ++x) {
                for (let y=0; y<dims.y; ++y) {
                    blocks.push(new Block(x,y,z));
                }
            }
        }

        return new Piece(blocks);
    }

    const game = new Game(pieceGenerator);
    assert.equal(game.getCementedBlocks().length, 0);
    assert.equal(game.getActiveBlocks().length, dims.x*dims.y*2 + 1);

    assert.ok(game.controls.drop());
    assert.ok(!game.isOver());

    let cem = game.getCementedBlocks();
    assert.equal(cem.length, 1);
    assert.equal(cem[0].x, 1);
    assert.equal(cem[0].y, 1);
    assert.equal(cem[0].z, 0);

    assert.ok(game.controls.drop());
    cem = game.getCementedBlocks();
    assert.equal(cem.length, 2 + dims.x*dims.y * 2);

    assert.equal(cem[0].x, 1);
    assert.equal(cem[0].y, 1);
    assert.equal(cem[0].z, 0);
});
