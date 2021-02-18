# wren-test
Testing using wren.io via the C API

# Build instructions

## Windows
    premake5 vs2019

## OSX
    premake xcode4

# TODO:
Items that need testing
- [x] class instantiation
- [x] class function calls
- [x] basic reflection
    - [ ] How do we find out what the parameter types for a function or accessor is?
- [ ] class getter / setter access
- [ ] class extension from foreign base class
- [ ] what does wrapping and exposing various structs to wren look like i.e. Vec3 
