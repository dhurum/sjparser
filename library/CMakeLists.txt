set(TARGET sjparser)

set(SJPARSER_INCLUDES_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(SRCS
  src/internals.cpp
  src/parsing_error.cpp
  src/yajl_parser.cpp
)

set(HDRS
  src/sjparser.h
  src/sjparser_impl.h
  src/internals.h
  src/internals_impl.h
  src/parsing_error.h
  src/yajl_parser.h
)

add_library(${TARGET} STATIC ${SRCS})

target_link_libraries(${TARGET} ${YAJL_LIB})
target_include_directories(${TARGET} PUBLIC ${SJPARSER_INCLUDES_DIR})

add_library(${TARGET}_shared SHARED ${SRCS})

target_link_libraries(${TARGET}_shared ${YAJL_LIB})
set_target_properties(${TARGET}_shared PROPERTIES OUTPUT_NAME ${TARGET})
target_include_directories(${TARGET}_shared PUBLIC ${SJPARSER_INCLUDES_DIR})

if(STANDALONE_BUILD)
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
endif()
