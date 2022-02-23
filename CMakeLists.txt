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

set(Arena_CPP_FLAGS $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -Werror
                    -fdiagnostics-color=always -fconcepts-diagnostics-depth=2>)
set(Arena_FETCH_CONTENT_OPTIONS GIT_PROGRESS ON $<$<CONFIG:RELEASE>:
                                GIT_SHALLOW ON>)

include(CTest)
include(FetchContent)

set(MEMORYCHECK_SUPPRESSIONS_FILE ${PROJECT_SOURCE_DIR}/test/python.supp)

FetchContent_Declare(
  box2d
  GIT_REPOSITORY https://github.com/erincatto/box2d
  GIT_TAG 9dc24a6fd4f32442c4bcf80791de47a0a7d25afb
  SOURCE_SUBDIR src ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG devel
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt
  GIT_TAG 3328c7e78bcf638a78d7a601d3780a10e7dd712c
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  gsl-lite
  GIT_REPOSITORY https://github.com/gsl-lite/gsl-lite
  GIT_TAG 4b5e9ab7474841fc2d7efc2e0064ef81785535d1
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  Little-Type-Library
  GIT_REPOSITORY https://github.com/qnope/Little-Type-Library
  GIT_TAG 97f09319aca070226b4674371fb4ae84fbb3b482
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11
  GIT_TAG 9aa676d38db4b18d9952c70486412d20c0213f13
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  sfml
  GIT_REPOSITORY https://github.com/SFML/sfml
  GIT_TAG fab2c93d7a56e85f60e26f065c051630ffff420e
  ${Arena_FETCH_CONTENT_OPTIONS})
FetchContent_Declare(
  units
  GIT_REPOSITORY https://github.com/mpusz/units
  GIT_TAG 3651e199bfba95b5e21e89df802488a4d079a62c
  SOURCE_SUBDIR src ${Arena_FETCH_CONTENT_OPTIONS})

FetchContent_MakeAvailable(gsl-lite)
set(gsl-lite_DIR ${gsl-lite_BINARY_DIR})
add_library(gsl-lite::gsl-lite ALIAS gsl-lite-v1)

if(Arena_BUILD_MODULE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()
set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
set(UNITS_BUILD_DOC OFF)
set(UNITS_USE_LIBFMT OFF)
find_package(Python3 COMPONENTS Interpreter Development)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/include)

if(Arena_BUILD_MODULE)
  pybind11_add_module(arena)
  target_link_libraries(arena PRIVATE Arena)
endif()

if(Arena_BUILD_TESTS)
  set(BUILD_TESTING ON)
  add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()
