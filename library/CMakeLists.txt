set(TARGET sjparser)

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
target_include_directories(sjparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(format)
add_format_target(sjparser)

if(SJPARSER_BUILD_SHARED_LIBRARY)
  add_library(sjparser_shared SHARED $<TARGET_PROPERTY:sjparser,SOURCES>)
  set_target_properties(sjparser_shared PROPERTIES OUTPUT_NAME sjparser)

  setup_compilation_options(sjparser_shared)
  target_link_libraries(sjparser_shared PUBLIC ${YAJL_LIB})
  target_include_directories(sjparser_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(STANDALONE_BUILD)
  install(TARGETS sjparser sjparser_shared DESTINATION lib/sjparser)
  get_target_property(HEADERS sjparser SJPARSER_HEADERS)
  install(FILES ${HEADERS} DESTINATION include/sjparser)

  add_subdirectory(check)
  add_subdirectory(documentation)
endif()
