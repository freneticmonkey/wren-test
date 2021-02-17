
class Script {

    construct new() {
        System.print("New Script!")

        _move_speed = 0
    }

    move_speed() { 
        _move_speed
    }

    move_speed(speed) {
        _move_speed = speed
    }    

    on_update(elapsed_time) {
        System.print("On Update")
        System.print("Elapsed Time: %(elapsed_time)")
        System.print("Move Speed: %(_move_speed)")
    }
}


