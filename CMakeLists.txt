cmake_minimum_required(VERSION 3.18)

message(STATUS "Finding ccache...")
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  message(STATUS "ccache found")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
else()
  message(WARNING "ccache not found")
endif()

project(
  Arena
  VERSION 0.0
  LANGUAGES CXX)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

set(Arena_ASSET_PATH ${PROJECT_SOURCE_DIR}/asset)
set(Arena_CPP_FLAGS $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -Werror
                    -fdiagnostics-color=always -fconcepts-diagnostics-depth=2>)
set(Arena_FETCH_CONTENT_OPTIONS GIT_PROGRESS ON)

include(CTest)
include(FetchContent)

set(MEMORYCHECK_SUPPRESSIONS_FILE ${PROJECT_SOURCE_DIR}/test/python.supp)

FetchContent_Declare(
  box2d
  GIT_REPOSITORY https://github.com/erincatto/box2d
  GIT_TAG main
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG devel
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  gsl-lite
  GIT_REPOSITORY https://github.com/gsl-lite/gsl-lite
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  Little-Type-Library
  GIT_REPOSITORY https://github.com/qnope/Little-Type-Library
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/StarQTius/pybind11
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  sfml
  GIT_REPOSITORY https://github.com/SFML/sfml
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  units
  GIT_REPOSITORY https://github.com/mpusz/units
  SOURCE_SUBDIR src ${Arena_FETCH_CONTENT_OPTIONS})

FetchContent_MakeAvailable(gsl-lite)
set(gsl-lite_DIR ${gsl-lite_BINARY_DIR})
add_library(gsl-lite::gsl-lite ALIAS gsl-lite-v1)

set(BOX2D_BUILD_UNIT_TESTS OFF)
set(BOX2D_BUILD_TESTBED OFF)
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
set(UNITS_BUILD_DOC OFF)
set(UNITS_USE_LIBFMT OFF)
find_package(Python3 COMPONENTS Interpreter Development)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/include)

if(Arena_BUILD_TESTS)
  set(BUILD_TESTING ON)
  add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()
