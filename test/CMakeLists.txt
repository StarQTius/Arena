FetchContent_MakeAvailable(pybind11 Catch2 Little-Type-Library)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

include(Arena)
include(CTest)
include(Catch)

# This target should invoke all testing target. Any testing target should
# therefore be made a dependency of this target.
add_custom_target(check)

#
# Unit test for the base library
#

set(TEST_LIST body.cpp field.cpp cup.cpp storage.cpp ray.cpp)

add_executable(cpp_tests main.cpp ${TEST_LIST})
target_link_libraries(cpp_tests PRIVATE Arena Catch2::Catch2WithMain
                                        pybind11::embed LTL)
target_compile_options(cpp_tests PRIVATE ${ARENA_CPP_FLAGS})
set_target_properties(cpp_tests PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
                                           CXX_VISIBILITY_PRESET hidden)
catch_discover_tests(
  cpp_tests
  PROPERTIES
  LABELS
  check_base
  EXTRA_ARGS
  --colour-mode
  ansi
  -r
  compact)

add_test_runner_target(check_base cpp_tests)
add_python_test("Base" check_base base.py)
add_python_test("Sail The World" check_base sail_the_world.py)

#
# Demo tests showcase the possible usage of Arena with the base library
#

add_demo_runner_target("Demo" demo_base demo.py)

foreach(TEST_FILE IN LISTS TEST_LIST)
  add_iwyu_target(${TEST_FILE} cpp_tests)
endforeach()
