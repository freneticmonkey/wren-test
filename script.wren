
class Script {

    construct new() {
        System.print("New Script!")

        _move_speed = 1
        _enabled = false
        _zomg = ":)"
    }
    
    construct new(speed) {
        _move_speed = speed
    }

    enabled() {
        _enabled
    }

    zomg() {
        _zomg
    }

    on_update(elapsed_time) {
        System.print("On Update")
        System.print("Elapsed Time: %(elapsed_time)")
        System.print("Move Speed: %(_move_speed)")
        _move_speed = _move_speed + elapsed_time
        System.print("New Move Speed: %(_move_speed)")
    }

    zomg(z) {
        _zomg = z
    }

    move_speed() { 
        _move_speed
    }

    move_speed(speed) {
        _move_speed = speed
    }  

    enabled(on) {
        _enabled = on
    }  
}


