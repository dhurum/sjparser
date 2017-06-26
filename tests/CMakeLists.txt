set(TARGET sjparser_tests)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

include_directories(${SJPARSER_INCLUDES} ${GTEST_INCLUDE_DIRS})

set(SRCS
  src/value.cpp
  src/array.cpp
  src/sarray.cpp
  src/object.cpp
  src/scustomobject.cpp
  src/sautoobject.cpp
  src/standalone_union.cpp
  src/object_union.cpp
  src/parsing_error.cpp
  src/yajl_parser.cpp
)

add_executable(${TARGET} ${SRCS})

target_link_libraries(${TARGET} ${SJPARSER_LIB} ${GTEST_BOTH_LIBRARIES} pthread)

add_test(sjparser ${TARGET})

if(CLANG_FORMAT_BIN)
  add_custom_target(format_tests
    COMMAND ${CLANG_FORMAT_BIN} -i ${SRCS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()
