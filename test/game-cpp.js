QUnit.test( "game basics", function( assert ) {
    const game = new Game(0);
    assert.ok( !game.isOver() );
    assert.equal( game.getScore(), 0 );
    game.drop();
    assert.ok( !game.isOver() );
    assert.ok( game.getScore() > 0 );

    assert.ok( game.getDimensions().x > 2 );
    assert.ok( game.getDimensions().y > 2 );
    assert.ok( game.getDimensions().z > 2 );

    assert.ok( game.rotate('X', 'CCW') );
    assert.ok( game.rotate('Y', 'CW') );
    assert.ok( game.rotate('y', 'cw') );
    assert.throws( () => game.rotate('sdf', 'CW') );
    assert.throws( () => game.rotate('X', 'aa') );
    assert.ok( game.moveXY(0,1) );
    assert.ok( game.moveXY(1,0) );
    assert.ok( !game.isOver() );
    game.drop();
    assert.ok( !game.isOver() );

    game.tick(10);
    game.tick(10);
    game.tick(10);
    game.tick(10);
    assert.ok( !game.isOver() );

    assert.equal( game.getActiveBlocks().length, 4 );
    assert.equal( game.getCementedBlocks().length, 8 );
    game.delete();

    const anotherGame = new Game(1);
    assert.ok( !anotherGame.isOver() );
    assert.equal( anotherGame.getScore(), 0 );
    anotherGame.delete();
});
