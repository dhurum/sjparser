set(TARGET sjparser_tests)

find_package(GTest REQUIRED)

include_directories(${SJPARSER_INCLUDES} ${GTEST_INCLUDE_DIRS})

add_executable(${TARGET}
  src/value.cpp
  src/array.cpp
  src/sarray.cpp
  src/object.cpp
  src/scustomobject.cpp
  src/sautoobject.cpp
  src/standalone_union.cpp
  src/object_union.cpp
)

target_link_libraries(${TARGET} ${SJPARSER_LIB} ${GTEST_BOTH_LIBRARIES})

add_test(sjparser ${TARGET})
