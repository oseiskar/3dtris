#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "piece.hpp"
#include "game.hpp"

TEST_CASE( "Pos3d", "[pos-3d]" ) {
    SECTION("sum") {
        Pos3d a { 1, 2, 3 }, b { 0, -2, 2 };
        Pos3d c = pos_methods::sum(a, b);
        REQUIRE( c.x == 1 );
        REQUIRE( c.y == 0 );
        REQUIRE( c.z == 5 );
    }
}

TEST_CASE( "Block", "[block]" ) {

    SECTION("translate") {
        Block b { Pos3d {1, 2, 3}, 100 };
        Block r = block_methods::translate(b, Pos3d { 1, -3, 5 });
        REQUIRE(r.material == 100);
        REQUIRE(r.pos.x == 2);
        REQUIRE(r.pos.y == -1);
        REQUIRE(r.pos.z == 8);
    }

    SECTION("rotate") {
        Block b { Pos3d {1, 2, 3}, 100 };
        Block xccw = block_methods::rotate(b, Rotation {
            Axis::X, Rotation::Direction::CCW });

        REQUIRE(xccw.material == 100);
        REQUIRE(xccw.pos.x == 1);
        REQUIRE(xccw.pos.y == -3);
        REQUIRE(xccw.pos.z == 2);

        REQUIRE(b.pos.x == 1);
        REQUIRE(b.pos.y == 2);
        REQUIRE(b.pos.z == 3);

        Block xcw = block_methods::rotate(b, Rotation {
            Axis::X, Rotation::Direction::CW });

        REQUIRE(xcw.material == 100);
        REQUIRE(xcw.pos.x == 1);
        REQUIRE(xcw.pos.y == 3);
        REQUIRE(xcw.pos.z == -2);
    }
}

TEST_CASE( "Piece", "[piece]" ) {

    SECTION("translate and rotate") {

        Piece p {
            Pos3d {1, 2, 3},
            {
                Block { Pos3d { 0, 0, 0 }, 100 },
                Block { Pos3d { 0, 1, 0 }, 101 }
            }
        };

        Piece r = p
            .translated(Pos3d{-1,0,1})
            .rotated(Rotation{Axis::X, Rotation::Direction::CCW});

        auto blocks = r.getBlocks();
        REQUIRE(blocks[0].pos.x == 0);
        REQUIRE(blocks[0].pos.y == 2);
        REQUIRE(blocks[0].pos.z == 4);
        REQUIRE(blocks[0].material == 100);
        REQUIRE(blocks[1].pos.x == 0);
        REQUIRE(blocks[1].pos.y == 2);
        REQUIRE(blocks[1].pos.z == 5);
        REQUIRE(blocks[1].material == 101);
    }

    SECTION("translatedBeyond") {

        Piece piece {
            Pos3d {1, 0, 0},
            {
                Block { Pos3d { 0, 1, 0 }, 100 },
                Block { Pos3d { 1, 0, 0 }, 101 }
            }
        };

        piece = piece.translatedBeyond(Axis::X, -1, 1);
        auto blocks = piece.getBlocks();
        REQUIRE( blocks[0].pos.x == 1 );
        REQUIRE( blocks[1].pos.x == 2 );

        piece = piece.translatedBeyond(Axis::X, 10, 1);
        blocks = piece.getBlocks();

        REQUIRE( blocks[0].pos.x == 10 );
        REQUIRE( blocks[0].pos.y == 1 );
        REQUIRE( blocks[1].pos.x == 11 );
        REQUIRE( blocks[1].pos.x == 11 );
        REQUIRE( blocks[1].pos.y == 0 );

        piece = piece.translatedBeyond(Axis::X, 1, -1);
        blocks = piece.getBlocks();
        REQUIRE( blocks[0].pos.x == 0 );
        REQUIRE( blocks[1].pos.x == 1 );
    }
}


TEST_CASE( "GameBox", "[game-box]" ) {
    GameBox box(Pos3d { 5, 6, 15 });

    SECTION("position isWithinBounds") {
        REQUIRE( box.contains(Pos3d { 0, 0, 0 }) );
        REQUIRE( box.contains(Pos3d { 4, 0, 0 }) );
        REQUIRE( !box.contains(Pos3d { 0, 6, 0 }) );
    }

    SECTION("translateToBounds") {
        Piece piece {
            Pos3d { 0, 0, 0 },
            {
                Block { Pos3d { -2, 0, 0 }, 10 },
                Block { Pos3d { 0, -1,  0}, 11 },
            }
        };

        REQUIRE( !box.contains(piece) );
        piece = piece.translated(Pos3d { 0, 10, 0 });
        REQUIRE( !box.contains(piece) );

        piece = box.translateToBounds(piece);
        REQUIRE(box.contains(piece));

        auto blocks = piece.getBlocks();
        REQUIRE( blocks[0].pos.x == 0 );
        REQUIRE( blocks[0].pos.y == 5);
        REQUIRE( blocks[1].pos.x == 2);
        REQUIRE( blocks[1].pos.y == 4);
    }

    SECTION("size") {
        REQUIRE( box.size() == 5*6*15 );
    }
}

TEST_CASE( "CementedBlockArray" "[cemented-block-array]") {
    SECTION("get and set blocks") {
        GameBox box(Pos3d { 5, 6, 15 });
        CementedBlockArray blocks(box);

        blocks.setBlock(Block{Pos3d { 0, 3, 2 }, 100});
        REQUIRE( !blocks.hasBlock(Pos3d{0, 3, 1}) );
        REQUIRE( blocks.hasBlock(Pos3d{0, 3, 2}) );

        auto ne = blocks.getNonEmptyBlocks();
        REQUIRE( ne.size() == 1 );
        REQUIRE( ne[0].pos.x == 0 );
        REQUIRE( ne[0].pos.y == 3 );
        REQUIRE( ne[0].pos.z == 2 );
        REQUIRE( ne[0].material == 100 );
    }

    SECTION("layers, not top") {
        GameBox box(Pos3d { 2, 2, 4 });
        CementedBlockArray blocks(box);

        blocks.setBlock(Block{Pos3d { 0, 0, 1 }, 10});
        blocks.setBlock(Block{Pos3d { 0, 1, 1 }, 10});
        blocks.setBlock(Block{Pos3d { 1, 0, 1 }, 10});
        blocks.setBlock(Block{Pos3d { 1, 1, 1 }, 10});

        REQUIRE( !blocks.isLayerFull(0) );
        REQUIRE( blocks.isLayerFull(1) );
        REQUIRE( !blocks.isLayerFull(2) );

        blocks.removeLayer(2);
        REQUIRE( !blocks.isLayerFull(0) );
        REQUIRE( blocks.isLayerFull(1) );
        REQUIRE( !blocks.isLayerFull(2) );

        blocks.removeLayer(0);
        REQUIRE( blocks.isLayerFull(0) );
        REQUIRE( !blocks.isLayerFull(1) );
        REQUIRE( blocks.hasBlock(Pos3d{0, 0, 0}) );
        REQUIRE( !blocks.hasBlock(Pos3d{0, 0, 1}) );
        REQUIRE( blocks.getNonEmptyBlocks().size() == 4 );

        blocks.removeLayer(0);
        REQUIRE( blocks.getNonEmptyBlocks().size() == 0 );
    }

    SECTION("top layer") {
        GameBox box(Pos3d { 2, 1, 3 });
        CementedBlockArray blocks(box);

        blocks.setBlock(Block{Pos3d { 0, 0, 2 }, 10});
        blocks.setBlock(Block{Pos3d { 1, 0, 2 }, 10});
        REQUIRE( blocks.isLayerFull(2) );
        REQUIRE( !blocks.isLayerFull(1) );
        REQUIRE( !blocks.isLayerFull(0) );
        REQUIRE( blocks.getNonEmptyBlocks().size() == 2 );
        blocks.removeLayer(2);
        REQUIRE( blocks.getNonEmptyBlocks().size() == 0 );
    }

    SECTION("piece fits and cement") {
        GameBox box(Pos3d { 2, 2, 3 });
        CementedBlockArray blocks(box);
        Piece piece {{
            Block { Pos3d{ 0, 0, 0 }, 1},
            Block { Pos3d{ 1, 0, 0 }, 2}
        }};

        blocks.setBlock(Block{Pos3d { 1, 0, 0 }, 10});
        REQUIRE( !blocks.pieceFits(piece) );

        piece = piece.translated(Pos3d { 0, 0, 1 });
        REQUIRE( blocks.pieceFits(piece) );
        REQUIRE( !blocks.pieceFits(piece.translated(Pos3d { -1, 0, 0} )) );
        blocks.cementPiece(piece);

        REQUIRE( !blocks.pieceFits(piece) );
        REQUIRE( blocks.hasBlock(Pos3d {1, 0, 1 }) );

        auto ne = blocks.getNonEmptyBlocks();
        REQUIRE( ne.size() == 3 );
        REQUIRE( ne[0].material == 10 );
        REQUIRE( ne[1].material == 1 );
        REQUIRE( ne[2].material == 2 );
    }
}
