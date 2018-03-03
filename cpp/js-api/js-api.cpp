#include <emscripten/bind.h>
#include "api.hpp"

EMSCRIPTEN_BINDINGS(game_builder) {
    emscripten::function("buildGame", &buildGame);
}

EMSCRIPTEN_BINDINGS(game_types) {
    emscripten::enum_<Axis>("Axis")
        .value("X", Axis::X)
        .value("Y", Axis::Y)
        .value("Z", Axis::Z);

    emscripten::enum_<RotationDirection>("RotationDirection")
        .value("CW", RotationDirection::CW)
        .value("CCW", RotationDirection::CCW);

    emscripten::value_object<Pos3d>("Pos3d")
        .field("x", &Pos3d::x)
        .field("y", &Pos3d::y)
        .field("z", &Pos3d::z);

    emscripten::value_object<Block>("Block")
        .field("pos", &Block::pos)
        .field("pieceId", &Block::pieceId);

     emscripten::register_vector<Block>("BlockVector");
}

EMSCRIPTEN_BINDINGS(game_api) {
  emscripten::class_<Game>("Game")
    .function("isOver", &Game::isOver)
    .function("getScore", &Game::getScore)
    .function("getDimensions", &Game::getDimensions)
    .function("tick", &Game::tick)
    .function("moveXY", &Game::moveXY)
    .function("rotate", &Game::rotate)
    .function("drop", &Game::drop)
    .function("getCementedBlocks", &Game::getCementedBlocks)
    .function("getActiveBlocks", &Game::getActiveBlocks);
}
