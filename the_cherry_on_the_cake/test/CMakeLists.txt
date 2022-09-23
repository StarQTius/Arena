include(CTest)

add_custom_target(check_coc COMMAND ctest -L check_coc --output-on-failure)
add_dependencies(check_coc _details)
add_dependencies(check check_coc)

add_test(
  NAME Binding
  COMMAND ${ARENA_PYTEST_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/binding.py
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
set_tests_properties(Binding PROPERTIES LABELS check_coc)
