FetchContent_MakeAvailable(box2d entt Little-Type-Library pybind11 units)

set(SOURCES_LIST
    body.cpp
    host.cpp
    field.cpp
    physics.cpp
    bot.cpp
    cup.cpp
    draw.cpp)

add_library(Arena STATIC ${SOURCES_LIST})
target_link_libraries(Arena PUBLIC box2d EnTT::EnTT pybind11::pybind11
                                   mp-units::mp-units LTL)
target_compile_definitions(Arena PUBLIC ARENA_C21)
target_compile_features(Arena PUBLIC cxx_std_20)
target_compile_options(Arena PRIVATE ${ARENA_CPP_FLAGS})
set_target_properties(Arena PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
                                       CXX_VISIBILITY_PRESET hidden)
