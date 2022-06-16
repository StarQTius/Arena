FetchContent_MakeAvailable(pybind11)

set(TEMPLATE_FILES __init__.py sail_the_world/__init__.py)

foreach(TEMPLATE_FILE IN LISTS TEMPLATE_FILES)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${TEMPLATE_FILE}.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${TEMPLATE_FILE} @ONLY)
endforeach()

pybind11_add_module(_details _details.cpp)
target_link_libraries(_details PRIVATE Arena)
set_target_properties(_details PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
                                          CXX_VISIBILITY_PRESET hidden)

add_custom_target(module DEPENDS _details)
