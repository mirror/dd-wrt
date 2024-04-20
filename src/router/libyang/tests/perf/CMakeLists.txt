set(format_sources
    ${format_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/perf.c
    PARENT_SCOPE)

add_executable(ly_perf ${CMAKE_CURRENT_SOURCE_DIR}/perf.c $<TARGET_OBJECTS:yangobj>)
set_target_properties(ly_perf PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
target_link_libraries(ly_perf ${CMAKE_THREAD_LIBS_INIT} ${PCRE2_LIBRARIES} ${CMAKE_DL_LIBS})
if (NOT WIN32)
    target_link_libraries(ly_perf m)
endif()

add_test(NAME ly_perf_1000 COMMAND ly_perf 1000 10)
add_test(NAME ly_perf_100000 COMMAND ly_perf 100000 3)
