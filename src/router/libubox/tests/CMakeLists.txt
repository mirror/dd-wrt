ADD_SUBDIRECTORY(cram)
ADD_SUBDIRECTORY(shunit2)

MACRO(ADD_UNIT_TEST name)
  ADD_EXECUTABLE(${name} ${name}.c)
  TARGET_LINK_LIBRARIES(${name} ubox blobmsg_json json_script ${json})
  TARGET_INCLUDE_DIRECTORIES(${name} PRIVATE ${PROJECT_SOURCE_DIR})
ENDMACRO(ADD_UNIT_TEST)

FILE(GLOB test_cases "test-*.c")
FOREACH(test_case ${test_cases})
  GET_FILENAME_COMPONENT(test_case ${test_case} NAME_WE)
  ADD_UNIT_TEST(${test_case})
  ADD_UNIT_TEST_SAN(${test_case})
ENDFOREACH(test_case)

IF(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  ADD_SUBDIRECTORY(fuzz)
ENDIF()
