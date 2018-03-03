#include "piece-generator.hpp"

namespace piece_generator {
    Piece prototypeToPiece( std::vector<Pos3d> pos, int material ) {
        std::vector<Block> blocks;
        for (Pos3d p : pos) blocks.push_back(Block {p, material});
        return Piece(blocks);
    }

    static const int NUMBER_OF_MATERIALS = 10;
}

PieceGenerator::PieceGenerator(const GameBox& gameBox_, int randomSeed)
:
    random(randomSeed),
    gameBox(gameBox_),
    prototypes {
        {
            Pos3d { -1,0,0 }, //  ##
            Pos3d { 0,0,0 },  // ##
            Pos3d { 0,1,0 },
            Pos3d { 1,1,0 }
        },
        {
            Pos3d { -1,0,0 }, // ####
            Pos3d { 0,0,0 },
            Pos3d { 1,0,0 },
            Pos3d { 2,0,0 }
        },
        {
            Pos3d { -1,0,0 }, // ###
            Pos3d { 0,0,0 },  //   #
            Pos3d { 1,0,0 },
            Pos3d { 1,1,0 }
        },
        {
            Pos3d { -1,0,0 }, // ###
            Pos3d { 0,0,0 },  //  #
            Pos3d { 1,0,0 },
            Pos3d { 0,1,0 }
        },
        {
            Pos3d { 0,0,0 }, // ##
            Pos3d { 0,1,0 },  // ##
            Pos3d { 1,0,0 },
            Pos3d { 1,1,0 }
        },
        {
            Pos3d { -1,0,0 }, // ##
            Pos3d { 0,0,0 },  //  o
            Pos3d { 0,1,1 },
            Pos3d { 0,1,0 }
        },
        {
            Pos3d { -1,0,0 }, // o#
            Pos3d { 0,0,0 },  //  #
            Pos3d { -1,0,1 },
            Pos3d { 0,1,0 }
        },
        {
            Pos3d { 0,0,0 }, // o#
            Pos3d { 1,0,0 }, // #
            Pos3d { 0,1,0 },
            Pos3d { 0,0,1 }
        },
    }
{}

Piece PieceGenerator::nextPiece() {
    std::uniform_int_distribution<> dist(0, prototypes.size()-1);
    return randomTransformation(
        piece_generator::prototypeToPiece(
            prototypes[dist(random)],
            nextMaterial())
                .translated(Pos3d{
                    gameBox.dims.x/2,
                    gameBox.dims.y/2,
                    gameBox.dims.z + 5
                }));
}

Piece PieceGenerator::randomTransformation(const Piece& original) {
    Piece piece = original;
    std::uniform_int_distribution<> lessThanFour(0, 4);
    for (Axis axis : { Axis::X, Axis::Y, Axis::Z} ) {
        for (int i = 0; i < lessThanFour(random); ++i) {
            piece = piece.rotated(Rotation{axis, RotationDirection::CCW});
        }
    }
    return gameBox.translateToBounds(piece);
}

int PieceGenerator::nextMaterial() {
    std::uniform_int_distribution<>
        dist(0, piece_generator::NUMBER_OF_MATERIALS-1);

    return dist(random);
}
