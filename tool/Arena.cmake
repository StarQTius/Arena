if(NOT TARGET iwyu)
  find_program(IWYU_PROGRAM include-what-you-use)
  if(NOT IWYU_PROGRAM)
    message(
      WARNING
        "`include-what-you-use` program not found, skipping inclusions check")
  else()
    set(IWYU_COMMAND
        ${IWYU_PROGRAM} -std=gnu++20 -fcolor-diagnostics -Xiwyu --no_fwd_decls
        -Xiwyu --error -Xiwyu --max_line_length=120 -Xiwyu
        --mapping_file=${PROJECT_SOURCE_DIR}/tool/mapping/pybind11.imp -Xiwyu
        --mapping_file=${PROJECT_SOURCE_DIR}/tool/mapping/entt.imp -Xiwyu
        --mapping_file=${PROJECT_SOURCE_DIR}/tool/mapping/libcpp.imp -Xiwyu
        --mapping_file=${PROJECT_SOURCE_DIR}/tool/mapping/ltl.imp
        CACHE INTERNAL "IWYU command run for each IWYU target")
  endif()

  add_custom_target(iwyu)
endif()

function(add_iwyu_target FILE_PATH DEPENDENCY)
  if(IWYU_PROGRAM)
    file(RELATIVE_PATH FILE_PATH_FROM_ROOT ${PROJECT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH})
    string(REPLACE "/" "_" TARGET_NAME ${FILE_PATH_FROM_ROOT})
    add_custom_command(
      OUTPUT ${CMAKE_BINARY_DIR}/${TARGET_NAME}.iwyu
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}
      COMMAND touch ${CMAKE_BINARY_DIR}/${TARGET_NAME}.iwyu
      COMMAND
        for INCLUDE_PATH in
        $<TARGET_PROPERTY:${DEPENDENCY},INCLUDE_DIRECTORIES>\\\; do
        INCLUDE_PATH_OPTION=\"$$INCLUDE_PATH_OPTION -I$$INCLUDE_PATH\"\\\;
        done\\\; ${IWYU_COMMAND} $$INCLUDE_PATH_OPTION
        ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH} 2> iwyu_log \\\; if [ $$? -ne 1
        ]\\\; then exit 0\\\; else cat iwyu_log\\\; exit 1\\\; fi
      COMMAND_EXPAND_LISTS
      IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}
      COMMENT "Checking ${FILE_PATH_FROM_ROOT} inclusions")
    add_custom_target(
      IWYU_${TARGET_NAME}
      DEPENDS ${CMAKE_BINARY_DIR}/${TARGET_NAME}.iwyu
      COMMENT "")
    set_target_properties(
      IWYU_${TARGET_NAME}
      PROPERTIES INCLUDE_DIRECTORIES
                 $<TARGET_PROPERTY:${DEPENDENCY},INCLUDE_DIRECTORIES>)

    add_dependencies(iwyu IWYU_${TARGET_NAME})
  endif()
endfunction()
