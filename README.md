# Ember
Ember is a quick and useful tool to turn binary files into C/C++ arrays.

## Inspiration
I started working on my own graphics and ui libraries.
During development I quickly realized that I had to manage a lot of files (images, shaders, etc.) during building.
The vision for Ember is to make working with non-source files easier.

## CMake Integration
```
# imports ember module
find_package(Ember MODULE REQUIRED)

# converts a binary file to a c++ std::array
ember_bake(<target> FILE <binary_file>)

# compiles shader and then calls ember_bake()
target_shaders(<target> SPIRV [<shader_file>...])
```

## Development
Ember is currently in very early stages of development and highly likely to change.
