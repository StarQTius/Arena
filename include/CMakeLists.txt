include(Arena)

target_include_directories(Arena PUBLIC ${PROJECT_SOURCE_DIR}/include)

set(STANDALONE_HEADERS
    arena.hpp
    binding/fetcher.hpp
    component/common.hpp
    component/ray.hpp
    concept.hpp
    environment.hpp
    physics.hpp)

foreach(HEADER IN LISTS STANDALONE_HEADERS)
  add_iwyu_target(arena/${HEADER} Arena)
endforeach()
