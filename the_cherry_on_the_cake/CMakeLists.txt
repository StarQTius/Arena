add_subdirectory(src)
add_subdirectory(include)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  add_subdirectory(arena ${ARENA_PACKAGE_DIR}/the_cherry_on_the_cake
                   EXCLUDE_FROM_ALL)
  add_subdirectory(test EXCLUDE_FROM_ALL)
endif()

add_library(Arena::TheCherryOnTheCake ALIAS TheCherryOnTheCake)
