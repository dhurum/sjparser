cmake_minimum_required(VERSION 2.8)

project(sjparser)

find_library(YAJL_LIB yajl)
if(NOT YAJL_LIB)
  message(FATAL_ERROR "Can't find yajl library")
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(--std=c++1y HAS_CXX14)
if(NOT HAS_CXX14)
  message(FATAL_ERROR "Compiler does not support c++14!")
endif()

set(NEED_TESTS ${WITH_TESTS})
set(NEED_COVERAGE FALSE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(NEED_TESTS TRUE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  set(NEED_TESTS TRUE)
  set(NEED_COVERAGE TRUE)
endif()

set(STANDALONE_BUILD TRUE)

if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message("The sjparser is a part of some project, disabling all but necessary targets.")
  set(STANDALONE_BUILD FALSE)
endif()

set(CMAKE_CXX_FLAGS
  ${CMAKE_CXX_FLAGS}
  -Werror
  -Wall
  -Wextra
  -Wpedantic
  --std=c++1y
)

set(CMAKE_CXX_FLAGS_DEBUG -g3)
set(CMAKE_CXX_FLAGS_COVERAGE
  -g3
  -O0
  --coverage
)

#transform list to string
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE ";" " " CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_COVERAGE}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CLANG_FORMAT_BIN clang-format)

add_subdirectory(library)

if(STANDALONE_BUILD)
  if(CLANG_FORMAT_BIN)
    add_custom_target(format DEPENDS format_library)
  else()
    message(STATUS "Can't find clang-format, no format target created")
  endif()

  if(NEED_TESTS)
    enable_testing()
    add_subdirectory(tests)

    if(CLANG_FORMAT_BIN)
      add_dependencies(format format_tests)
    endif()
  endif()

  if(NEED_COVERAGE)
    add_subdirectory(coverage)
  endif()
endif()
