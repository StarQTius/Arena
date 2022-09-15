cmake_minimum_required(VERSION 3.21)

# Find `ccache` utility if possible
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

# Set the configuration file
file(
  WRITE .git/config
  "[include]
    path = ../.github/config
")

# Generate compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Get project version from git tags
execute_process(COMMAND git describe --tags
                OUTPUT_VARIABLE ARENA_GIT_TAG_VERSION)
string(REGEX REPLACE ".*v([0-9]+.[0-9]+.[0-9]+).*" "\\1" ARENA_GIT_TAG_VERSION
                     ${ARENA_GIT_TAG_VERSION})

project(
  Eurobot-Arena
  VERSION ${ARENA_GIT_TAG_VERSION}
  LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/tool)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF)
set(ARENA_CPP_FLAGS
    -Wall
    -Wextra
    -Werror
    $<$<CXX_COMPILER_ID:GNU>:
    -fdiagnostics-color=always
    -fconcepts-diagnostics-depth=2>
    $<$<CXX_COMPILER_ID:Clang>:
    -Wno-missing-braces
    -Wno-unneeded-internal-declaration
    -fcolor-diagnostics>)
set(ARENA_BASE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(ARENA_BASE_MODULE_DIR ${PROJECT_SOURCE_DIR}/arena)
set(ARENA_PACKAGE_DIR ${PROJECT_BINARY_DIR}/arena)

# Fetch dependencies from GitHub
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(
  box2d
  GIT_REPOSITORY https://github.com/erincatto/box2d
  GIT_TAG main
  SOURCE_SUBDIR src
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG v3.0.1
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt
  GIT_TAG v3.10.1
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  expected
  GIT_REPOSITORY https://github.com/TartanLlama/expected
  GIT_TAG v1.0.0
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  gsl-lite
  GIT_REPOSITORY https://github.com/gsl-lite/gsl-lite
  GIT_TAG v0.40.0
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  Little-Type-Library
  GIT_REPOSITORY https://github.com/StarQTius/Little-Type-Library
  GIT_TAG Arena
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11
  GIT_TAG v2.9.2
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  units
  GIT_REPOSITORY https://github.com/mpusz/units
  SOURCE_SUBDIR src
  GIT_PROGRESS ON
  GIT_SHALLOW ON)

# Configure dependencies
FetchContent_MakeAvailable(gsl-lite)
set(gsl-lite_DIR ${gsl-lite_BINARY_DIR})
add_library(gsl-lite::gsl-lite ALIAS gsl-lite-v1)
set(LTL_CODE_COVERAGE OFF)
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
set(UNITS_BUILD_DOC OFF)
set(UNITS_USE_LIBFMT OFF)

# Hint Python installation location in manylinux
if(DEFINED ENV{ARENA_MANYLINUX_PYTHON3_INCLUDE_DIR})
  # These lines are only needed to help CMake find Python in the manylinux
  # docker image. We expect Python3 not to be found, but it is okay since we are
  # giving CMake enough hints to find the necessary resources for building the
  # wheel.
  set(Python3_INCLUDE_DIR $ENV{ARENA_MANYLINUX_PYTHON3_INCLUDE_DIR})
  find_package(Python3 COMPONENTS Interpreter Development)
  set(Python3_FOUND TRUE)
else()
  # For desktop distribution of Linux, Python is easily found.
  find_package(Python3 COMPONENTS Interpreter Development)
endif()

set(ARENA_PYTEST_COMMAND ${Python3_EXECUTABLE} -B -m pytest --color=yes -p
                         no:cacheprovider)

add_subdirectory(src)
add_subdirectory(include)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  add_subdirectory(arena EXCLUDE_FROM_ALL)
  add_subdirectory(test EXCLUDE_FROM_ALL)
endif()

add_library(Arena::Arena ALIAS Arena)
add_subdirectory(the_cherry_on_the_cake)
