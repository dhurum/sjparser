set(TARGET sjparser)

FIND_PROGRAM(GCOV_BIN gcov)
FIND_PROGRAM(LCOV_BIN lcov)
FIND_PROGRAM(GENHTML_BIN genhtml)

if(NOT GCOV_BIN)
  message(FATAL_ERROR "Can't find gcov")
endif()

if(NOT LCOV_BIN)
  message(FATAL_ERROR "Can't find lcov")
endif()

if(NOT GENHTML_BIN)
  message(FATAL_ERROR "Can't find genhtml")
endif()

set(BASE_INFO_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.base.info)
set(TEST_INFO_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.test.info)

add_custom_command(OUTPUT ${BASE_INFO_FILE} ${TEST_INFO_FILE}
  COMMAND ${LCOV_BIN} --capture --initial --directory . --output-file ${BASE_INFO_FILE}
  COMMAND ctest
  COMMAND ${LCOV_BIN} --capture --directory . --output-file ${TEST_INFO_FILE}
  WORKING_DIRECTORY  ${CMAKE_BINARY_DIR}
  DEPENDS sjparser_tests
)

add_custom_target(coverage ALL
  COMMAND ${LCOV_BIN} --add-tracefile ${BASE_INFO_FILE} --add-tracefile ${TEST_INFO_FILE} --output ${TARGET}.info
  COMMAND ${LCOV_BIN} --remove ${TARGET}.info '*/tests/*' '/usr/*' --output-file ${TARGET}.clean.info
  COMMAND ${GENHTML_BIN} --output-directory report ${TARGET}.clean.info
  DEPENDS ${BASE_INFO_FILE} ${TEST_INFO_FILE}
)
