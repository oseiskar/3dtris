// requires cpp/bin/js/game.js

function vectorToJsArray(vec) {
    var arr = [];
    for (var i=0; i<vec.size(); ++i) {
        arr.push(vec.get(i));
    }
    vec.delete();
    return arr;
}

class Game {
    constructor(randomSeed) {
        this._game = Module.buildGame(randomSeed);
        const methods = [
            'isOver',
            'getScore',
            'getDimensions',
            'tick',
            'moveXY',
            'drop',
            'delete' // emscripten
        ];
        methods.forEach(method => {
            this[method] = this._game[method].bind(this._game);
        });
    }

    rotate(axis, dir) {
        const ax = Module.Axis[axis.toUpperCase()];
        if (!ax) throw new Error("invalid axis "+axis);
        const d = Module.RotationDirection[dir.toUpperCase()];
        if (!d) throw new Error("invalid rotation direction "+dir);
        return this._game.rotate(ax, d);
    }

    getCementedBlocks() {
        return Game._vectorToJsArray(this._game.getCementedBlocks());
    }

    getActiveBlocks() {
        return Game._vectorToJsArray(this._game.getActiveBlocks());
    }

    getAllBlocks() {
        return Game._vectorToJsArray(this._game.getAllBlocks());
    }
}

Game._vectorToJsArray = function (vec) {
    var arr = [];
    for (var i=0; i<vec.size(); ++i) {
        arr.push(vec.get(i));
    }
    vec.delete();
    return arr;
};
