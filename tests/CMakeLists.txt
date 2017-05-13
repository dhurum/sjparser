set(TARGET sjparser_tests)

find_package(GTest REQUIRED)

include_directories(${SJPARSER_INCLUDES} ${GTEST_INCLUDE_DIRS})

add_executable(${TARGET}
  value_parser.cpp
  array_parser.cpp
  sarray_parser.cpp
  object_parser.cpp
  scustomobject_parser.cpp
  sautoobject_parser.cpp
  standalone_union_parser.cpp
  object_union_parser.cpp
)

target_link_libraries(${TARGET} ${SJPARSER_LIB} ${GTEST_BOTH_LIBRARIES})

add_test(sjparser ${TARGET})
