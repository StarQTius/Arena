FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

include(Catch)

add_executable(run_tests module.cpp main.cpp host.cpp component.cpp entity.cpp
                         render.cpp)
target_link_libraries(run_tests PRIVATE Arena Catch2::Catch2WithMain
                                        pybind11::embed LTL)
target_compile_definitions(run_tests PRIVATE ARENA_C21 ARENA_EMBED)
target_compile_options(run_tests PRIVATE ${ARENA_CPP_FLAGS})
set_target_properties(
  run_tests
  PROPERTIES CXX_STANDARD 20
             INTERPROCEDURAL_OPTIMIZATION ON
             CXX_VISIBILITY_PRESET hidden
             POSITION_INDEPENDENT_CODE ON)

catch_discover_tests(run_tests TEST_SPEC)
catch_discover_tests(run_tests TEST_SPEC [integration])
