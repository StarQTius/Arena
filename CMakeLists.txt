cmake_minimum_required(VERSION 3.13)

project(
  Cassiope
  VERSION 0.0
  LANGUAGES CXX)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/box2d/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/Catch2)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/entt)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/Little-Type-Library)

find_package(Python COMPONENTS Interpreter Development)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pybind11)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/test)
