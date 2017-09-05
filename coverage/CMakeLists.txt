find_program(GCOV_BIN gcov)
find_program(LCOV_BIN lcov)
find_program(GENHTML_BIN genhtml)

if(NOT GCOV_BIN)
  message(FATAL_ERROR "Can't find gcov")
endif()

if(NOT LCOV_BIN)
  message(FATAL_ERROR "Can't find lcov")
endif()

if(NOT GENHTML_BIN)
  message(FATAL_ERROR "Can't find genhtml")
endif()

set(BASE_INFO_FILE ${CMAKE_CURRENT_BINARY_DIR}/sjparser.base.info)
set(TEST_INFO_FILE ${CMAKE_CURRENT_BINARY_DIR}/sjparser.test.info)

add_custom_command(OUTPUT ${BASE_INFO_FILE} ${TEST_INFO_FILE}
  COMMAND ${LCOV_BIN} --zerocounters --directory .
  COMMAND ${LCOV_BIN} --capture --initial --directory . --output-file
  ${BASE_INFO_FILE}
  COMMAND ctest
  COMMAND ${LCOV_BIN} --capture --directory . --output-file ${TEST_INFO_FILE}
  WORKING_DIRECTORY  ${CMAKE_BINARY_DIR}
  DEPENDS sjparser_tests
)

add_custom_target(coverage ALL
  COMMAND ${LCOV_BIN} --add-tracefile ${BASE_INFO_FILE} --add-tracefile
  ${TEST_INFO_FILE} --output sjparser.info
  COMMAND ${LCOV_BIN} --remove sjparser.info '*/tests/*' '/usr/*' --output-file
  sjparser.clean.info
  COMMAND ${GENHTML_BIN} --output-directory report sjparser.clean.info
  DEPENDS ${BASE_INFO_FILE} ${TEST_INFO_FILE}
)
