foreign class Vec3 {
    construct new() {}
    construct new(nx, ny , nz) {
        x(nx)
        y(ny)
        z(nz)
    }
    foreign x()
    foreign x(v)

    foreign y()
    foreign y(v)

    foreign z()
    foreign z(v)

    foreign toString()
}

foreign class Transform {
    construct new() {}
    construct new(np) {
        pos(np)
    }
    foreign pos()
    foreign pos(p)
}

foreign class Node {
    construct new() {}
    foreign name()
    foreign name(n)
}