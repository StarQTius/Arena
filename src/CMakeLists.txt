add_library(Arena
  host.cpp
  field.cpp
  physics.cpp
  bot.cpp
  cup.cpp)
target_link_libraries(Arena
  PUBLIC
    box2d
    EnTT::EnTT
    pybind11::pybind11
    mp-units::mp-units
  PRIVATE
    LTL)
target_compile_definitions(Arena PRIVATE ARENA_C21)
set_target_properties(Arena PROPERTIES CXX_STANDARD 20 INTERPROCEDURAL_OPTIMIZATION ON CXX_VISIBILITY_PRESET hidden)
if(MSVC)
  target_compile_options(Arena PRIVATE /W4 /WX)
else()
  target_compile_options(Arena PRIVATE -Wall -Wextra -Werror)
endif()
