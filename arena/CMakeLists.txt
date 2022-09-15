FetchContent_MakeAvailable(pybind11)

include(Arena)

set(TEMPLATE_FILES __init__.py storage_base.py sail_the_world/__init__.py)

foreach(TEMPLATE_FILE IN LISTS TEMPLATE_FILES)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${TEMPLATE_FILE}.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${TEMPLATE_FILE} @ONLY)
endforeach()

pybind11_add_module(_details _details.cpp sail_the_world/_details.cpp
                    binder/host.cpp)
target_link_libraries(_details PRIVATE Arena)
set_target_properties(_details PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
                                          CXX_VISIBILITY_PRESET hidden)
target_compile_features(_details PUBLIC cxx_std_20)
target_compile_options(_details PRIVATE ${ARENA_CPP_FLAGS} -DENTT_API_EXPORT)
target_include_directories(_details PRIVATE ${PROJECT_SOURCE_DIR}/src)

add_custom_target(module DEPENDS _details)

get_target_property(MODULE_SOURCES _details SOURCES)
foreach(SOURCE_FILE IN LISTS MODULE_SOURCES)
  add_iwyu_target(${SOURCE_FILE} _details)
endforeach()

set(STANDALONE_HEADERS
    box2d.hpp
    common.hpp
    component_ref.hpp
    physics.hpp
    python.hpp
    utility.hpp
    binder/_entity.hpp
    binder/component.hpp
    binder/ctor.hpp
    binder/def.hpp
    binder/doc.hpp
    binder/fetcher.hpp
    binder/internal_component.hpp
    binder/normalize.hpp
    binder/property.hpp
    binder/static_def.hpp
    binder/traits.hpp)

foreach(HEADER IN LISTS STANDALONE_HEADERS)
  add_iwyu_target(${HEADER} _details)
endforeach()
