add_library(src host.cpp)
target_include_directories(src PUBLIC ../include)
target_link_libraries(src PUBLIC box2d EnTT::EnTT LTL pybind11::pybind11 pybind11::headers)
set_target_properties(src PROPERTIES CXX_STANDARD 20 INTERPROCEDURAL_OPTIMIZATION ON CXX_VISIBILITY_PRESET hidden)
