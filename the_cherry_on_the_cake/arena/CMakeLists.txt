include(Arena)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in
               ${CMAKE_CURRENT_BINARY_DIR}/__init__.py @ONLY)

add_arena_submodule(_details_coc _details.cpp)
target_link_libraries(_details_coc PUBLIC TheCherryOnTheCake)
