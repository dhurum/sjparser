find_package(GTest REQUIRED)

add_executable(sjparser_tests
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
  src/map.cpp
  src/ignore.cpp
)

setup_compilation_options(sjparser_tests)
target_link_libraries(sjparser_tests PUBLIC sjparser GTest::GTest GTest::Main)

include(format)
add_format_target(sjparser_tests)

add_test(sjparser sjparser_tests)
