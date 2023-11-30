FILE(GLOB test_cases "test-*.c")

MACRO(ADD_FUZZER_TEST name)
  ADD_EXECUTABLE(${name} ${name}.c)
  TARGET_COMPILE_OPTIONS(${name} PRIVATE -g -O1 -fno-omit-frame-pointer -fsanitize=fuzzer,address,leak,undefined)
  TARGET_INCLUDE_DIRECTORIES(${name} PRIVATE ${PROJECT_SOURCE_DIR})
  TARGET_LINK_OPTIONS(${name} PRIVATE -stdlib=libc++ -fsanitize=fuzzer,address,leak,undefined)
  TARGET_LINK_LIBRARIES(${name} ubox blobmsg_json json_script ${json})
  ADD_TEST(
    NAME ${name}
    COMMAND ${name} -max_len=256 -timeout=10 -max_total_time=300 ${CMAKE_CURRENT_SOURCE_DIR}/corpus
  )
ENDMACRO(ADD_FUZZER_TEST)

FOREACH(test_case ${test_cases})
  GET_FILENAME_COMPONENT(test_case ${test_case} NAME_WE)
  ADD_FUZZER_TEST(${test_case})
ENDFOREACH(test_case)
