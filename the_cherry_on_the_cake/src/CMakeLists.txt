include(Arena)

set(SOURCE_LIST cake_layer.cpp)

add_arena_library(TheCherryOnTheCake ${SOURCE_LIST})

target_link_libraries(TheCherryOnTheCake PUBLIC Arena)

foreach(SOURCE IN LISTS SOURCE_LIST)
  add_iwyu_target(${SOURCE} TheCherryOnTheCake)
endforeach()
