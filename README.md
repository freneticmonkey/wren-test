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
    - [x] How do we find out what the parameter types for a function or accessor is?
- [x] class getter / setter access
- [x] class extension from foreign base class - Don't extend, just inject
- [x] what does wrapping and exposing various structs to wren look like i.e. Vec3
- [x] Define 'built-in' classes in separate module and import
