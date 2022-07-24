FetchContent_MakeAvailable(box2d entt expected Little-Type-Library pybind11
                           units)

include(Arena)

set(SOURCES_LIST
    body.cpp
    bot.cpp
    cup.cpp
    draw.cpp
    environment.cpp
    field.cpp
    host.cpp
    physics.cpp
    storage.cpp)

set(STANDALONE_HEADERS forward.hpp)

add_library(Arena STATIC ${SOURCES_LIST})
target_link_libraries(Arena PUBLIC box2d EnTT::EnTT pybind11::pybind11
                                   mp-units::mp-units LTL expected)
target_compile_features(Arena PUBLIC cxx_std_20)
target_compile_options(Arena PRIVATE ${ARENA_CPP_FLAGS})
set_target_properties(Arena PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
                                       CXX_VISIBILITY_PRESET hidden)

foreach(SOURCE_FILE IN LISTS SOURCES_LIST)
  add_iwyu_target(${SOURCE_FILE} Arena)
endforeach()
foreach(HEADER IN LISTS STANDALONE_HEADERS)
  add_iwyu_target(${HEADER} Arena)
endforeach()
