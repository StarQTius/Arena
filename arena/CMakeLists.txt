FetchContent_MakeAvailable(pybind11)

include(Arena)

set(TEMPLATE_FILES __init__.py sail_the_world/__init__.py)

foreach(TEMPLATE_FILE IN LISTS TEMPLATE_FILES)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${TEMPLATE_FILE}.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${TEMPLATE_FILE} @ONLY)
endforeach()

pybind11_add_module(_details common.cpp _details.cpp
                    sail_the_world/_details.cpp)
target_link_libraries(_details PRIVATE Arena)
set_target_properties(_details PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
                                          CXX_VISIBILITY_PRESET hidden)
target_compile_features(_details PUBLIC cxx_std_20)
target_compile_options(_details PRIVATE ${ARENA_CPP_FLAGS} -Wno-deprecated-copy)
target_include_directories(_details PRIVATE ${PROJECT_SOURCE_DIR}/src)

add_custom_target(module DEPENDS _details)

get_target_property(MODULE_SOURCES _details SOURCES)
foreach(SOURCE_FILE IN LISTS MODULE_SOURCES)
  add_iwyu_target(${SOURCE_FILE} _details)
endforeach()

set(HEADERS box2d.hpp binder.hpp physics.hpp python.hpp utility.hpp
            with_units.hpp)
foreach(HEADER IN LISTS HEADERS)
  add_iwyu_target(${HEADER} _details)
endforeach()
