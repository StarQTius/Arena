cmake_minimum_required(VERSION 3.13)

project(
  Arena
  VERSION 0.0
  LANGUAGES CXX)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/lib/Catch2/extras)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

set(MEMORYCHECK_SUPPRESSIONS_FILE ${PROJECT_SOURCE_DIR}/test/python.supp)
include(CTest)
include(FetchContent)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/box2d/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/Catch2)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/entt)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/Little-Type-Library)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/gsl-lite)
set(gsl-lite_DIR ${CMAKE_BINARY_DIR}/lib/gsl-lite)
add_library(gsl-lite::gsl-lite ALIAS gsl-lite-v1)
set(UNITS_USE_LIBFMT OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/units/src)

find_package(Python COMPONENTS Interpreter Development)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pybind11)

set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
FetchContent_Declare(SFML GIT_REPOSITORY https://github.com/SFML/sfml)
FetchContent_MakeAvailable(SFML)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/test)
