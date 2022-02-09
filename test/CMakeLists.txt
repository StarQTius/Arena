include(Catch)

add_executable(run_tests
  module.cpp
  main.cpp
  host.cpp
  component.cpp
  entity.cpp
  render.cpp)
target_link_libraries(run_tests PRIVATE
  Arena
  Catch2::Catch2WithMain
  pybind11::embed
  LTL)
target_compile_definitions(run_tests PRIVATE ARENA_C21 ARENA_EMBED)
set_target_properties(run_tests PROPERTIES CXX_STANDARD 20 INTERPROCEDURAL_OPTIMIZATION ON CXX_VISIBILITY_PRESET hidden)
if(MSVC)
  target_compile_options(run_tests PRIVATE /W4 /WX)
else()
  target_compile_options(run_tests PRIVATE -Wall -Wextra -Werror -fdiagnostics-color=always)
endif()

catch_discover_tests(run_tests TEST_SPEC)
catch_discover_tests(run_tests TEST_SPEC [.integration])
