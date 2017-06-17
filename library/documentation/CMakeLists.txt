find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
  message(STATUS "Can't find doxygen, no documentation target created")
  return()
endif()

set(CUT_README ${CMAKE_CURRENT_BINARY_DIR}/Readme.md)
set(CUTTER ${CMAKE_CURRENT_SOURCE_DIR}/cut_nodoc.sh)

add_custom_command(OUTPUT ${CUT_README}
  COMMAND ${CUTTER} ${CMAKE_SOURCE_DIR}/Readme.md > ${CUT_README}
  DEPENDS ${CMAKE_SOURCE_DIR}/Readme.md
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  @ONLY
)

add_custom_target(documentation
  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS ${CUT_README}
)
