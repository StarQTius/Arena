## Requirements

### Running the python module
- Python 3.8

### Compiling the module from source
- GCC 11
- CMake 3.10
- [EnTT](https://github.com/skypjack/entt)
- [Little-Type-Library](https://github.com/qnope/Little-Type-Library)
- [pybind11](https://github.com/pybind/pybind11)
- [units](https://github.com/mpusz/units)

### Running the tests
- [Catch2](https://github.com/catchorg/Catch2)

## Embedding custom components or systems

Calling Python code too often from the simulation may result in heavily degraded performance. In that case, it is possible to embed your components and systems directly in the simulation.

The code for the simulation is written in C++ with the pybind11 library, and the source can be compiled through CMake integration. To understand how to embed a new components and systems, you must understand how to use the [different libraries](#markdown-header-compiling-the-module-from-source) involved.
