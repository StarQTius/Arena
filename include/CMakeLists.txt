include(Arena)

target_include_directories(Arena PUBLIC ${PROJECT_SOURCE_DIR}/include)

set(STANDALONE_HEADERS
    arena.hpp
    component/common.hpp
    component/ray.hpp
    component/stackable.hpp
    traits/common.hpp
    traits/crtp.hpp
    traits/invocable.hpp
    traits/template.hpp
    traits/type.hpp)

foreach(HEADER IN LISTS STANDALONE_HEADERS)
  add_iwyu_target(arena/${HEADER} Arena)
endforeach()
