FIND_PROGRAM(CLANG_TIDY_BIN clang-tidy)

if(NOT CLANG_TIDY_BIN)
  message(STATUS "Can't find clang-tidy, no check target created")
  return()
endif()

set(DUMMY ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)

add_custom_command(OUTPUT ${DUMMY}
  COMMAND ./generate_dummy.sh ${SJPARSER_SOURCE_DIR} ${DUMMY} ${HDRS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(dummy EXCLUDE_FROM_ALL ${DUMMY})

set(CHECKS
  *
  -cppcoreguidelines-pro-type-reinterpret-cast
  -readability-implicit-bool-cast
  -google-runtime-int
  -google-explicit-constructor
  -cppcoreguidelines-special-member-functions
  # For some reason, it does not play well with comments before pragma once
  -llvm-header-guard
)

string(REPLACE ";" "," CHECKS "${CHECKS}")

add_custom_target(check_srcs
  COMMAND ${CLANG_TIDY_BIN} -p ${CMAKE_BINARY_DIR}
  -checks=${CHECKS} ${SRCS}
  WORKING_DIRECTORY  ${SJPARSER_SOURCE_DIR}
)

add_custom_target(check_hdrs
  COMMAND ${CLANG_TIDY_BIN} -p ${CMAKE_BINARY_DIR}
  -checks=${CHECKS} --header-filter=.* ${DUMMY}
  DEPENDS ${DUMMY}
)

add_custom_target(check
  DEPENDS check_srcs check_hdrs
)
