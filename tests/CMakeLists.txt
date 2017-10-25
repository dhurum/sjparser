set(TARGET sjparser_tests)

find_package(GTest REQUIRED)

add_executable(${TARGET}
  src/value.cpp
  src/array.cpp
  src/sarray.cpp
  src/object.cpp
  src/scustomobject.cpp
  src/sautoobject.cpp
  src/standalone_union.cpp
  src/standalone_sunion.cpp
  src/embedded_union.cpp
  src/embedded_sunion.cpp
  src/parsing_error.cpp
  src/yajl_parser.cpp
  src/dispatcher.cpp
)

setup_compilation_options(${TARGET})
target_link_libraries(${TARGET} PUBLIC sjparser GTest::GTest GTest::Main)

include(format)
add_format_target(${TARGET})

add_test(sjparser ${TARGET})
