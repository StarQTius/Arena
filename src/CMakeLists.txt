FetchContent_MakeAvailable(box2d entt Little-Type-Library pybind11 units sfml)

add_library(
  Arena STATIC
  body.cpp
  host.cpp
  field.cpp
  physics.cpp
  bot.cpp
  cup.cpp
  SFMLDebugDraw.cpp)
target_sources(Arena INTERFACE module.cpp)
target_link_libraries(Arena PUBLIC box2d EnTT::EnTT pybind11::pybind11
                                   mp-units::mp-units sfml-graphics LTL)
target_compile_definitions(Arena PUBLIC ARENA_C21)
set_target_properties(
  Arena
  PROPERTIES CXX_STANDARD 20
             INTERPROCEDURAL_OPTIMIZATION ON
             CXX_VISIBILITY_PRESET hidden
             POSITION_INDEPENDENT_CODE ON)
target_compile_options(Arena PRIVATE ${Arena_CPP_FLAGS})
