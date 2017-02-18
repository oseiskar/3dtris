QUnit.test( "test block rotation", function( assert ) {

    let testBlock = new Block(1,2,3);

    let rotatedXCCW = testBlock.rotated('x', 1);

    assert.equal( rotatedXCCW.x, 1 );
    assert.equal( rotatedXCCW.y, -3 );
    assert.equal( rotatedXCCW.z, 2 );

    let rotatedXCW = testBlock.rotated('x', -1);

    assert.equal( rotatedXCW.x, 1 );
    assert.equal( rotatedXCW.y, 3 );
    assert.equal( rotatedXCW.z, -2 );
});

QUnit.test( "test piece rotation and translation", function( assert ) {

    let piece = new Piece([
        new Block(0,0,0),
        new Block(1,0,0),
        new Block(0,1,0),
        new Block(0,2,0)
    ]);

    piece.translate(0,4,0);
    piece.rotate('x', 1);

    let blocks = piece.getBlocks();

    assert.equal(blocks[0].x, 0);
    assert.equal(blocks[0].y, 4);
    assert.equal(blocks[0].z, 0);

    assert.equal(blocks[2].x, 0);
    assert.equal(blocks[2].y, 4);
    assert.equal(blocks[2].z, 1);
});


QUnit.test( "test translateBeyond", function( assert ) {

    let piece = new Piece([
        new Block(0,1,0),
        new Block(1,0,0)
    ]);

    piece.translate(1,0,0);

    assert.ok(!piece.translateBeyond('x', -1, 1));
    let blocks = piece.getBlocks();
    assert.equal(blocks[0].x, 1);
    assert.equal(blocks[1].x, 2);

    assert.ok(piece.translateBeyond('x', 10, 1));
    blocks = piece.getBlocks();

    assert.equal(blocks[0].x, 10);
    assert.equal(blocks[0].y, 1);

    assert.equal(blocks[1].x, 11);
    assert.equal(blocks[1].y, 0);

    assert.ok(piece.translateBeyond('x', 1, -1));
    blocks = piece.getBlocks();
    assert.equal(blocks[0].x, 0);
    assert.equal(blocks[1].x, 1);
});
