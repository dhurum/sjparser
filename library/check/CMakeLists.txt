find_program(CLANG_TIDY_BIN clang-tidy)

if(NOT CLANG_TIDY_BIN)
  message(STATUS "Can't find clang-tidy, no check target created")
  return()
endif()

set(DUMMY ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)

add_custom_command(OUTPUT ${DUMMY}
  COMMAND ./generate_dummy.sh ${DUMMY}
  $<TARGET_PROPERTY:sjparser,SJPARSER_HEADERS>
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND_EXPAND_LISTS
)

add_executable(dummy EXCLUDE_FROM_ALL ${DUMMY})
target_link_libraries(dummy PRIVATE sjparser)
setup_compilation_options(dummy)

set(CHECKS
  *
  # Dealing with C api requires lots of reinterpret casts
  -cppcoreguidelines-pro-type-reinterpret-cast
  # Google style specific check
  -google-readability-todo
  # For some reason, it does not play well with comments before pragma once
  -llvm-header-guard
  # clang-tidy and clang-format seems to disagree
  -llvm-include-order
  # Project-specific checks, not useful in this case
  -fuchsia-default-arguments
  -fuchsia-trailing-return
)

string(REPLACE ";" "," CHECKS "${CHECKS}")

get_target_property(SJPARSER_SOURCE_DIR sjparser SOURCE_DIR)

add_custom_target(check_srcs
  COMMAND ${CLANG_TIDY_BIN} -p ${CMAKE_BINARY_DIR}
  -checks=${CHECKS} $<TARGET_PROPERTY:sjparser,SOURCES>
  WORKING_DIRECTORY  ${SJPARSER_SOURCE_DIR}
  COMMAND_EXPAND_LISTS
)

add_custom_target(check_hdrs
  COMMAND ${CLANG_TIDY_BIN} -p ${CMAKE_BINARY_DIR}
  -checks=${CHECKS} -warnings-as-errors=* --header-filter=.* ${DUMMY}
  DEPENDS ${DUMMY}
)

add_custom_target(check
  DEPENDS check_srcs check_hdrs
)
