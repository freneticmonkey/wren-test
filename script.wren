import "core" for Vec3, Node, Transform

class Component {
    pos { _pos }
    pos=(v) { _pos=v }
    node { _node }
    node=(v) { _node=v }

    transform { _transform }
    transform=(v) { _transform=v }
}

class BehaviourComponent is Component {
    construct new() {
        pos = Vec3.new()
        node = Node.new()
        node.name("hello")
        
        transform = Transform.new(Vec3.new(1,2,3))
    }

    update(value) {
        pos.x(value)

        System.print("Pos Vec3: %(pos.toString())")
        System.print("node Name: %(node.name())")
        var p = transform.pos()
        System.print("Transform: Pos Vec3: %(p.toString())")
    }
}
    

class Script is Component {
    construct new() {
        System.print("New Script!")

        _move_speed = 1
        _enabled = false
        _zomg = ":)"
    }
    
    construct new(speed) {
        _move_speed = speed
    }

    enabled=(on) {
        _enabled = on
    } 

    enabled {
        return _enabled
    }

    zomg=(z) {
        _zomg = z
    }

    zomg {
        return _zomg
    }

    move_speed {
        return _move_speed
    }

    move_speed=(speed) {
        _move_speed = speed
    }

    on_update(elapsed_time) {
        System.print("On Update")
        System.print("Elapsed Time: %(elapsed_time)")
        System.print("Move Speed: %(_move_speed)")
        _move_speed = _move_speed + elapsed_time
        System.print("New Move Speed: %(_move_speed)")
    }
}


