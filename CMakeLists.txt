cmake_minimum_required(VERSION 3.21)

message(STATUS "Finding ccache...")
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  message(STATUS "ccache found")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
else()
  message(WARNING "ccache not found")
endif()

execute_process(COMMAND git describe --tags
                OUTPUT_VARIABLE ARENA_GIT_TAG_VERSION)
string(REGEX REPLACE ".*v([0-9]+.[0-9]+.[0-9]+).*" "\\1" ARENA_GIT_TAG_VERSION
                     ${ARENA_GIT_TAG_VERSION})
project(
  Eurobot-Arena
  VERSION ${ARENA_GIT_TAG_VERSION}
  LANGUAGES CXX)

set(ARENA_CPP_FLAGS $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -Werror
                    -fdiagnostics-color=always -fconcepts-diagnostics-depth=2>)

include(CTest)
include(FetchContent)

set(FETCHCONTENT_QUIET OFF)
set(MEMORYCHECK_SUPPRESSIONS_FILE ${PROJECT_SOURCE_DIR}/test/python.supp)

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
  GIT_TAG devel
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  gsl-lite
  GIT_REPOSITORY https://github.com/gsl-lite/gsl-lite
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  Little-Type-Library
  GIT_REPOSITORY https://github.com/qnope/Little-Type-Library
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_Declare(
  units
  GIT_REPOSITORY https://github.com/mpusz/units
  SOURCE_SUBDIR src
  GIT_PROGRESS ON
  GIT_SHALLOW ON)

FetchContent_MakeAvailable(gsl-lite)
set(gsl-lite_DIR ${gsl-lite_BINARY_DIR})
add_library(gsl-lite::gsl-lite ALIAS gsl-lite-v1)

if(ARENA_BUILD_MODULE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()
set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
set(UNITS_BUILD_DOC OFF)
set(UNITS_USE_LIBFMT OFF)

if(DEFINED ENV{ARENA_MANYLINUX_PYTHON3_INCLUDE_DIR})
  # Python3 installation hinting for manylinux We expect Python3 not to be
  # found, but it is okay since we are giving CMake enough hints to find the
  # necessary resources for building the wheel.
  set(Python3_INCLUDE_DIR $ENV{ARENA_MANYLINUX_PYTHON3_INCLUDE_DIR})
  find_package(Python3 COMPONENTS Interpreter Development)
  set(Python3_FOUND TRUE)
else()
  # Standard Python3 detection
  find_package(Python3 COMPONENTS Interpreter Development)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/include)

if(ARENA_BUILD_MODULE)
  pybind11_add_module(arena)
  target_link_libraries(arena PRIVATE Arena)
endif()

if(ARENA_BUILD_TESTS)
  set(BUILD_TESTING ON)
  add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()
