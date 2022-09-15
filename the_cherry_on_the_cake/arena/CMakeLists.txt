include(Arena)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in
               ${CMAKE_CURRENT_BINARY_DIR}/__init__.py @ONLY)
add_arena_module(_coc_details _coc_details.cpp)
target_link_libraries(_coc_details PRIVATE TheCherryOnTheCake)
add_iwyu_target(_coc_details.cpp _coc_details)
