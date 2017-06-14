find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
  message(STATUS "Can't find doxygen, no documentation target created")
  return()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  @ONLY
)

add_custom_target(documentation
  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
