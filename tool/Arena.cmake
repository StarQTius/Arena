FetchContent_MakeAvailable(units)

if(NOT TARGET iwyu)
  find_program(IWYU_PROGRAM include-what-you-use)
  if(NOT IWYU_PROGRAM)
    message(
      WARNING
        "`include-what-you-use` program not found, skipping inclusions check")
  else()
    set(IWYU_COMMAND
        ${IWYU_PROGRAM} -std=gnu++20 -fcolor-diagnostics -DARENA_IWYU -Xiwyu
        --no_fwd_decls -Xiwyu --error -Xiwyu --max_line_length=120 -Xiwyu
        --mapping_file=${PROJECT_SOURCE_DIR}/tool/mapping/Arena.imp -Xiwyu
        --mapping_file=${PROJECT_SOURCE_DIR}/tool/mapping/pybind11.imp -Xiwyu
        --mapping_file=${PROJECT_SOURCE_DIR}/tool/mapping/entt.imp -Xiwyu
        --mapping_file=${PROJECT_SOURCE_DIR}/tool/mapping/libcpp.imp -Xiwyu
        --prefix_header_includes=keep -include units/isq/si/prefixes.h -include
        ltl/Range/Range.h -include ltl/Range/BaseIterator.h
        CACHE INTERNAL "IWYU command run for each IWYU target")
  endif()

  add_custom_target(iwyu_headers)
  add_custom_target(iwyu)
endif()

function(add_iwyu_target FILE_PATH DEPENDENCY)
  if(IWYU_PROGRAM)
    # Find the relative path of the file from project root to make a shorter
    # target name and comment logs
    file(RELATIVE_PATH FILE_PATH_FROM_ROOT ${PROJECT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH})
    string(REPLACE "/" "_" TARGET_NAME ${FILE_PATH_FROM_ROOT})

    # Add the command and its corresponding target
    add_custom_command(
      OUTPUT ${CMAKE_BINARY_DIR}/${TARGET_NAME}.iwyu
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}
              ${PROJECT_SOURCE_DIR}/tool/mapping/entt.imp
              ${PROJECT_SOURCE_DIR}/tool/mapping/libcpp.imp
              ${PROJECT_SOURCE_DIR}/tool/mapping/pybind11.imp
      COMMAND touch ${CMAKE_BINARY_DIR}/${TARGET_NAME}.iwyu
      COMMAND
        for INCLUDE_PATH in
        $<TARGET_PROPERTY:${DEPENDENCY},INCLUDE_DIRECTORIES>\\\; do
        INCLUDE_PATH_OPTION=\"$$INCLUDE_PATH_OPTION -isystem$$INCLUDE_PATH\"\\\;
        done\\\; ${IWYU_COMMAND} -I${CMAKE_CURRENT_SOURCE_DIR}
        $$INCLUDE_PATH_OPTION $$COMPILE_DEF_OPTION
        ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH} 2> iwyu_log \\\; if [ $$? -ne 1
        ]\\\; then exit 0\\\; else cat iwyu_log && printf %\"$$\(tput cols\)\"s
        | tr ' ' '='\\\; exit 1\\\; fi
      COMMAND_EXPAND_LISTS
      IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}
      COMMENT "Checking ${FILE_PATH_FROM_ROOT} inclusions")
    add_custom_target(
      IWYU_${TARGET_NAME}
      DEPENDS ${CMAKE_BINARY_DIR}/${TARGET_NAME}.iwyu
      COMMENT "")
    add_custom_target(IWYU_${TARGET_NAME}_group DEPENDS IWYU_${TARGET_NAME})

    # Add the include paths so CMake can find the file dependencies
    set_target_properties(
      IWYU_${TARGET_NAME}
      PROPERTIES INCLUDE_DIRECTORIES
                 $<TARGET_PROPERTY:${DEPENDENCY},INCLUDE_DIRECTORIES>)

    # If it's a source, add it to `iwyu` target and make it depends on
    # `iwyu_headers` If it's a standalone header, add it to `iwyu_headers`
    # Standalone headers are then checked first in order to reduce checking time
    string(FIND ${TARGET_NAME} .hpp IS_HEADER)
    if(IS_HEADER EQUAL -1)
      add_dependencies(iwyu IWYU_${TARGET_NAME}_group)
      add_dependencies(IWYU_${TARGET_NAME}_group iwyu_headers)
    else()
      add_dependencies(iwyu_headers IWYU_${TARGET_NAME}_group)
    endif()
  endif()
endfunction()

function(add_arena_library LIBRARY_NAME SOURCE_LIST)
  add_library(${LIBRARY_NAME} STATIC ${SOURCE_LIST})
  target_link_libraries(
    ${LIBRARY_NAME} PUBLIC box2d EnTT::EnTT pybind11::pybind11
                           mp-units::mp-units LTL expected)
  target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_20)
  target_compile_options(${LIBRARY_NAME} PRIVATE ${ARENA_CPP_FLAGS})
  set_target_properties(
    ${LIBRARY_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
                               CXX_VISIBILITY_PRESET hidden)
  target_include_directories(${LIBRARY_NAME} PRIVATE ${ARENA_BASE_SOURCE_DIR})
endfunction()

function(add_arena_module MODULE_NAME SOURCE_LIST)
  pybind11_add_module(${MODULE_NAME} ${SOURCE_LIST})
  set_target_properties(
    ${MODULE_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
                              CXX_VISIBILITY_PRESET hidden)
  target_compile_features(${MODULE_NAME} PUBLIC cxx_std_20)
  target_compile_options(${MODULE_NAME} PRIVATE ${ARENA_CPP_FLAGS})
  target_include_directories(${MODULE_NAME} PRIVATE ${ARENA_BASE_SOURCE_DIR}
                                                    ${ARENA_BASE_MODULE_DIR})
endfunction()
