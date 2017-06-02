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
install(
  FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sjparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sjparser_impl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sjparser_internals.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sjparser_internals_impl.h
  DESTINATION
  include/sjparser
)
