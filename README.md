This library is a C++ powered Python package for simulating playing areas according to the Eurobot rules. The aim is to provide simulated environment for testing your robots or training your agents.

This package also comes as a C++ library if you need to speed up your simulation even more.

> **warning**
> This project is currently on development. A lot of features might be removed or changed without notice.

# Howto

## Installation

### As a Python package using pip

This library is not yet available on Pypi, but you can install it from source via Github.

``` bash
pip3 install git+https://github.com/StarQTius/Arena
```

### As a C++ library using CMake

Fetch the project from Github (e.g. via [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)) with the method of your choice and link your project against the target(s) you need.

## The library structure

The basic element for using this library can be found in the `arena` module (`Arena` if you are using CMake).

Specific components for the different editions of the Eurobot contest can be found in their respective submodules (e.g. `arena.the_cherry_on_the_cake`). Their corresponding CMake targets have similar names writtent in camel case (e.g. `TheCherryOnTheCake`).

## Quickstart

Arena is based on the [ECS](https://en.wikipedia.org/wiki/Entity_component_system) pattern which is implemented by frameworks and libraries like [Unity](https://unity.com/) and [EnTT](https://github.com/skypjack/entt) (in fact, Arena uses EnTT for its ECS implementation). Therefore, entities in your environment can be added new features by attaching components to them.

``` python:test/snippet.py:[4-20]
```

Logics can be added to your entities as coroutines thanks to the `Host` component. Arena will automatically fetch the requested components according to the parameter annotations of the async function.

You can advance the state of the environment by a given timestep with the `Environment.step` method.
