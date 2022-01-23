## Requirements

### Running the python module
- Python 3.8

### Compiling the module from source
Libraries needed:
- GCC 11
- CMake 3.10
- [Box2D](https://github.com/erincatto/box2d)
- [EnTT](https://github.com/skypjack/entt)
- [gsl-lite](https://github.com/gsl-lite/gsl-lite)
- [Little-Type-Library](https://github.com/qnope/Little-Type-Library)
- [pybind11](https://github.com/pybind/pybind11)
- [units](https://github.com/mpusz/units)

To make these libraries available to CMake, use the `git init` and `git update` commands.

### Running the tests
Libraries needed:
- [Catch2](https://github.com/catchorg/Catch2)

Likewise, use `git init` and `git update` to install thise library. Once installed, you can compile and run the test suites by running `check` and `check_integration`.

## Embedding custom components or systems

Calling Python code too often from the simulation may result in heavily degraded performance. In that case, it is possible to embed your components and systems directly in the simulation.

The code for the simulation is written in C++ with the pybind11 library, and the source can be compiled through CMake integration. To understand how to embed a new components and systems, you must understand how to use the [different involved libraries](#markdown-header-compiling-the-module-from-source).
