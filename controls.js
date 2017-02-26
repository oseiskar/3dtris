function executeAction(controls, key) {

    switch (key) {

        case "q":
            return controls.rotate('y', -1);

        case "e":
            return controls.rotate('y', 1);

        case "2":
            return controls.rotate('x', 1);

        case "x":
            return controls.rotate('x', -1);

        case "r":
            return controls.rotate('z', -1);

        case "t":
            return controls.rotate('z', 1);

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
            return controls.drop();
    }
    return null;
}
