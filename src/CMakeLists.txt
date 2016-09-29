set(TARGET sjparser)

set(SJPARSER_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}
  CACHE INTERNAL ""
)
set(SJPARSER_LIB
  ${TARGET}
  CACHE INTERNAL ""
)

include_directories(${SJPARSER_INCLUDES})

add_library(${TARGET} STATIC
  sjparser.cpp
)

add_library(${TARGET}_shared SHARED
  sjparser.cpp
)
set_target_properties(${TARGET}_shared PROPERTIES OUTPUT_NAME ${TARGET})

target_link_libraries(${TARGET} ${YAJL_LIB})

install(TARGETS ${TARGET} ${TARGET}_shared DESTINATION lib/sjparser)
install(
  FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/sjparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sjparser_impl.h
  DESTINATION
  include/sjparser
)
