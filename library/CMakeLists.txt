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

set(SRCS
  src/internals.cpp
  src/parse_error.cpp
  src/yajl_parser.cpp
)

set(HDRS
  src/sjparser.h
  src/sjparser_impl.h
  src/internals.h
  src/internals_impl.h
  src/parse_error.h
  src/yajl_parser.h
)

add_library(${TARGET} STATIC ${SRCS})

target_link_libraries(${TARGET} ${YAJL_LIB})

add_library(${TARGET}_shared SHARED ${SRCS})

target_link_libraries(${TARGET}_shared ${YAJL_LIB})
set_target_properties(${TARGET}_shared PROPERTIES OUTPUT_NAME ${TARGET})

install(TARGETS ${TARGET} ${TARGET}_shared DESTINATION lib/sjparser)
install(FILES ${HDRS} DESTINATION include/sjparser)

set(SJPARSER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(check)
add_subdirectory(documentation)

if(CLANG_FORMAT_BIN)
  add_custom_target(format_library
    COMMAND ${CLANG_FORMAT_BIN} -i ${SRCS} ${HDRS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()
