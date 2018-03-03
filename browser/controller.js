"use strict";
/* globals Game, window */

class GameController {
    constructor() {
        const randSeed = Math.round(Math.random() * 0xffffffff);
        this.game = new Game(randSeed);
    }

    notifyChanged() {
        if (this.changedCallback) {
            this.changedCallback(this.game);
        }
    }

    nextFrame(dtMilliseconds) {
        if (this.game.tick(dtMilliseconds))
            this.notifyChanged();
    }

    run() {
        let prevTime;
        const that = this;

        function animationStep(timestamp) {
            if (!prevTime) prevTime = timestamp;
            const dt = timestamp - prevTime;

            that.nextFrame(dt);

            if (!that.game.isOver()) {
                window.requestAnimationFrame(animationStep);
            }
            prevTime = timestamp;
        }

        window.requestAnimationFrame(animationStep);
    }

    onKeyDown(e) {
        const action = executeAction(this.game, e.key);
        if (action) this.notifyChanged();
    }
}
