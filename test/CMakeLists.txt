FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

include(Arena)
include(CTest)
include(Catch)

set(TEST_LIST host.cpp field.cpp cup.cpp)

set(PYTEST_COMMAND ${Python3_EXECUTABLE} -B -m pytest --color=yes -p
                   no:cacheprovider)

add_executable(run_tests main.cpp ${TEST_LIST})
target_link_libraries(run_tests PRIVATE Arena Catch2::Catch2WithMain
                                        pybind11::embed LTL)
target_compile_options(run_tests PRIVATE ${ARENA_CPP_FLAGS})
set_target_properties(run_tests PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
                                           CXX_VISIBILITY_PRESET hidden)

catch_discover_tests(
  run_tests
  PROPERTIES
  LABELS
  check
  EXTRA_ARGS
  --colour-mode
  ansi
  -r
  compact)
add_test(
  NAME base_tests
  COMMAND ${PYTEST_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/base.py
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(
  NAME sail_the_world_tests
  COMMAND ${PYTEST_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/sail_the_world.py
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(
  NAME demo_tests
  COMMAND ${PYTEST_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/demo.py
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(check COMMAND ctest -L check --output-on-failure)
add_dependencies(check module run_tests)
set_tests_properties(base_tests PROPERTIES LABELS check)
set_tests_properties(sail_the_world_tests PROPERTIES LABELS check)

add_custom_target(demo COMMAND ctest -L demo)
set_tests_properties(demo_tests PROPERTIES LABELS demo)

foreach(TEST_FILE IN LISTS TEST_LIST)
  add_iwyu_target(${TEST_FILE} run_tests)
endforeach()
