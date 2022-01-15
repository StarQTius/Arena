add_executable(run_tests main.cpp host.cpp component.cpp)
target_link_libraries(run_tests PUBLIC src Catch2::Catch2WithMain pybind11::embed)
set_target_properties(run_tests PROPERTIES CXX_STANDARD 20 INTERPROCEDURAL_OPTIMIZATION ON CXX_VISIBILITY_PRESET hidden)
