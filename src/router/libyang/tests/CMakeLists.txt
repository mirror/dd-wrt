# Correct RPATH usage on OS X
set(CMAKE_MACOSX_RPATH TRUE)

configure_file("${PROJECT_SOURCE_DIR}/tests/tests_config.h.in" "${PROJECT_BINARY_DIR}/tests/tests_config.h" ESCAPE_QUOTES @ONLY)
include_directories(SYSTEM ${CMOCKA_INCLUDE_DIR})
include_directories(${PROJECT_BINARY_DIR}/tests/)

function(ly_add_utest)
    cmake_parse_arguments(ADDTEST "" "NAME;WRAP" "SOURCES" ${ARGN})
    set(TEST_NAME utest_${ADDTEST_NAME})

    foreach(TEST_SOURCE ${ADDTEST_SOURCES})
        list(APPEND TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SOURCE})
    endforeach()

    add_executable(${TEST_NAME} ${TEST_SOURCES} $<TARGET_OBJECTS:yangobj>)
    target_compile_definitions(${TEST_NAME} PRIVATE LIBYANG_BUILD)

    # Set common attributes of all tests
    set_target_properties(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
    target_link_libraries(${TEST_NAME} ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${PCRE2_LIBRARIES} ${CMAKE_DL_LIBS})
    if (NOT WIN32)
        target_link_libraries(${TEST_NAME} m)
    else()
        target_link_libraries(${TEST_NAME} ${COMPAT_WIN_LIBRARIES})
    endif()
    if (NOT APPLE)
        if (ADDTEST_WRAP)
            set_target_properties(${TEST_NAME} PROPERTIES LINK_FLAGS "${ADDTEST_WRAP}")
        endif()
    endif()

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT "MALLOC_CHECK_=3")

    if(ENABLE_VALGRIND_TESTS)
        add_test(${TEST_NAME}_valgrind valgrind --leak-check=full --show-leak-kinds=all --suppressions=${PROJECT_SOURCE_DIR}/tests/ld.supp --error-exitcode=1 ${CMAKE_BINARY_DIR}/tests/${TEST_NAME})
    endif()
endfunction()

if(ENABLE_TESTS)
    add_subdirectory(plugins)
    add_subdirectory(utests)
    if(NOT WIN32)
        add_subdirectory(style)
        add_subdirectory(fuzz)
    endif()
    add_subdirectory(yanglint)
    add_subdirectory(yangre)
endif()
if(ENABLE_PERF_TESTS)
    add_subdirectory(perf)
endif()

set(format_sources ${format_sources} PARENT_SCOPE)
