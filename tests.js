QUnit.test( "test block rotation", function( assert ) {

    let testBlock = new Block(1,2,3);

    let rotatedXCCW = rotate(testBlock, 'x', 1);

    assert.equal( rotatedXCCW.x, 1 );
    assert.equal( rotatedXCCW.y, -3 );
    assert.equal( rotatedXCCW.z, 2 );

    let rotatedXCW = rotate(testBlock, 'x', -1);

    assert.equal( rotatedXCW.x, 1 );
    assert.equal( rotatedXCW.y, 3 );
    assert.equal( rotatedXCW.z, -2 );
});
