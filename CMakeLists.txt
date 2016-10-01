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

set(CMAKE_CXX_FLAGS
  ${CMAKE_CXX_FLAGS}
  -Werror
  -Wall
  -Wextra
  -Wpedantic
  --std=c++1y
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  enable_testing()
else()
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -O2)
endif()

set (CMAKE_CXX_FLAGS_DEBUG -g3)

#transform list to string
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_subdirectory(tests)
endif()
