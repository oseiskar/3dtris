function executeAction(controls, key) {

    switch (key) {

        case "q":
            return controls.rotate('y', 'cw');

        case "e":
            return controls.rotate('y', 'ccw');

        case "2":
            return controls.rotate('x', 'ccw');

        case "x":
            return controls.rotate('x', 'cw');

        case "r":
            return controls.rotate('z', 'cw');

        case "t":
            return controls.rotate('z', 'ccw');

        case "ArrowLeft":
        case "a":
            return controls.moveXY(-1,0);

        case "ArrowRight":
        case "d":
            return controls.moveXY(1,0);

        case "ArrowUp":
        case "w":
            return controls.moveXY(0,-1);

        case "ArrowDown":
        case "s":
            return controls.moveXY(0,1);

        case " ":
            $("#help-text").hide(1000);
            controls.drop();
            return true;
    }
    return null;
}
