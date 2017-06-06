set(TARGET sjparser)

set(SJPARSER_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  CACHE INTERNAL ""
)
set(SJPARSER_LIB
  ${TARGET}
  CACHE INTERNAL ""
)

include_directories(${SJPARSER_INCLUDES})

set(SRCS src/sjparser_internals.cpp)

set(HDRS
  src/sjparser.h
  src/sjparser_impl.h
  src/sjparser_internals.h
  src/sjparser_internals_impl.h
)

add_library(${TARGET} STATIC
  ${SRCS}
)
target_link_libraries(${TARGET} ${YAJL_LIB})

add_library(${TARGET}_shared SHARED
  ${SRCS}
)
target_link_libraries(${TARGET}_shared ${YAJL_LIB})
set_target_properties(${TARGET}_shared PROPERTIES OUTPUT_NAME ${TARGET})


install(TARGETS ${TARGET} ${TARGET}_shared DESTINATION lib/sjparser)
install(FILES ${HDRS} DESTINATION include/sjparser)

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

add_custom_target(check
  COMMAND clang-tidy -p ${CMAKE_BINARY_DIR}
  -checks=${CHECKS} --header-filter=src/ ${SRCS} ${HDRS}
  WORKING_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}
)
