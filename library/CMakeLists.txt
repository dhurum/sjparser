find_library(YAJL_LIB yajl)
if(NOT YAJL_LIB)
  message(FATAL_ERROR "Can't find yajl library")
endif()

add_library(sjparser STATIC
  sjparser/internals.cpp
  sjparser/parsing_error.cpp
  sjparser/yajl_parser.cpp
)
set_property(TARGET sjparser PROPERTY SJPARSER_HEADERS
  sjparser/sjparser.h
  sjparser/sjparser_impl.h
  sjparser/internals.h
  sjparser/internals_impl.h
  sjparser/parsing_error.h
  sjparser/yajl_parser.h
)
set_target_properties(sjparser PROPERTIES OUTPUT_NAME sjparser_static)

setup_compilation_options(sjparser)

target_link_libraries(sjparser PUBLIC ${YAJL_LIB})

target_include_directories(sjparser PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

include(format)
add_format_target(sjparser)

add_library(sjparser::sjparser ALIAS sjparser)

if(SJPARSER_BUILD_SHARED_LIBRARY)
  add_library(sjparser_shared SHARED $<TARGET_PROPERTY:sjparser,SOURCES>)
  set_target_properties(sjparser_shared PROPERTIES OUTPUT_NAME sjparser)

  setup_compilation_options(sjparser_shared)

  target_link_libraries(sjparser_shared PUBLIC ${YAJL_LIB})

  target_include_directories(sjparser_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
  )

  add_library(sjparser::sjparser_shared ALIAS sjparser_shared)
endif()

if(STANDALONE_BUILD)
  install(TARGETS sjparser sjparser_shared
    EXPORT sjparser-targets
    DESTINATION lib/sjparser
  )
  get_target_property(HEADERS sjparser SJPARSER_HEADERS)
  install(FILES ${HEADERS} DESTINATION include/sjparser)

  set(PACKAGE_LOCATION lib/cmake/sjparser)
  install(FILES sjparser-config.cmake DESTINATION ${PACKAGE_LOCATION})
  install(EXPORT sjparser-targets
    NAMESPACE sjparser::
    DESTINATION ${PACKAGE_LOCATION}
  )

  add_subdirectory(check)
  add_subdirectory(documentation)
endif()
