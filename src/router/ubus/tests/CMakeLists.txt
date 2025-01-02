ADD_SUBDIRECTORY(cram)

MACRO(ADD_UNIT_TEST name)
  ADD_EXECUTABLE(${name} ${name}.c)
  TARGET_LINK_LIBRARIES(${name} ubox blobmsg_json json_script ${json})
  TARGET_INCLUDE_DIRECTORIES(${name} PRIVATE ${PROJECT_SOURCE_DIR})
ENDMACRO(ADD_UNIT_TEST)

MACRO(ADD_UNIT_TEST_CPP name)
  ADD_EXECUTABLE(${name} ${name}.cpp)
  TARGET_LINK_LIBRARIES(${name} ubox blobmsg_json json_script ${json})
  TARGET_INCLUDE_DIRECTORIES(${name} PRIVATE ${PROJECT_SOURCE_DIR})
ENDMACRO(ADD_UNIT_TEST_CPP)

FILE(GLOB test_cases "test-*.c")
FOREACH(test_case ${test_cases})
  GET_FILENAME_COMPONENT(test_case ${test_case} NAME_WE)
  ADD_UNIT_TEST(${test_case})
  ADD_UNIT_TEST_SAN(${test_case})
ENDFOREACH(test_case)

ENABLE_LANGUAGE(CXX)
ADD_COMPILE_OPTIONS($<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11>)
ADD_UNIT_TEST_CPP(test-cplusplus)
ADD_TEST(NAME cplusplus COMMAND test-cplusplus)

IF(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  ADD_SUBDIRECTORY(fuzz)
ENDIF()
