# generate test code coverage report

# check that coverage tools are available - always use before GEN_COVERAGE
macro(GEN_COVERAGE_ENABLE ENABLE_TESTS)
    # make into normal variable
    set(TESTS_ENABLED ${ENABLE_TESTS})

    set(GEN_COVERAGE_ENABLED ON)
    if(NOT TESTS_ENABLED)
        message(WARNING "You cannot generate coverage when tests are disabled. Enable test by additing parameter -DENABLE_TESTS=ON or run cmake with Debug build target.")
        set(GEN_COVERAGE_ENABLED OFF)
    endif()

    if(GEN_COVERAGE_ENABLED)
        find_program(PATH_GCOV NAMES gcov)
        if(NOT PATH_GCOV)
            message(WARNING "gcov executable not found! Disabling building code coverage report.")
            set(GEN_COVERAGE_ENABLED OFF)
        endif()
    endif()

    if(GEN_COVERAGE_ENABLED)
        find_program(PATH_LCOV NAMES lcov)
        if(NOT PATH_LCOV)
            message(WARNING "lcov executable not found! Disabling building code coverage report.")
            set(GEN_COVERAGE_ENABLED OFF)
        endif()
    endif()

    if(GEN_COVERAGE_ENABLED)
        find_program(PATH_GENHTML NAMES genhtml)
        if(NOT PATH_GENHTML)
            message(WARNING "genhtml executable not found! Disabling building code coverage report.")
            set(GEN_COVERAGE_ENABLED OFF)
        endif()
    endif()

    if(GEN_COVERAGE_ENABLED)
        if(NOT CMAKE_COMPILER_IS_GNUCC)
            message(WARNING "Compiler is not gcc! Coverage may break the tests!")
        endif()

        execute_process(
            COMMAND bash "-c" "${CMAKE_C_COMPILER} --version | head -n1 | sed \"s/.* (.*) \\([0-9]\\+.[0-9]\\+.[0-9]\\+ .*\\)/\\1/\""
            OUTPUT_VARIABLE GCC_VERSION_FULL
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND bash "-c" "${PATH_GCOV} --version | head -n1 | sed \"s/.* (.*) \\([0-9]\\+.[0-9]\\+.[0-9]\\+ .*\\)/\\1/\""
            OUTPUT_VARIABLE GCOV_VERSION_FULL
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(NOT GCC_VERSION_FULL STREQUAL GCOV_VERSION_FULL)
            message(WARNING "gcc and gcov versions do not match! Generating coverage may fail with errors.")
        endif()

        # add specific required compile flags
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    endif()
endmacro()

# tests are always expected to be in ${CMAKE_SOURCE_DIR}/tests
function(GEN_COVERAGE MATCH_TEST_REGEX EXCLUDE_TEST_REGEX)
    if(NOT GEN_COVERAGE_ENABLED)
        return()
    endif()

    # destination
    set(COVERAGE_DIR        "${CMAKE_BINARY_DIR}/code_coverage/")
    set(COVERAGE_FILE_RAW   "${CMAKE_BINARY_DIR}/coverage_raw.info")
    set(COVERAGE_FILE_CLEAN "${CMAKE_BINARY_DIR}/coverage_clean.info")

    # test match/exclude
    if(MATCH_TEST_REGEX)
        set(MATCH_TEST_ARGS -R \"${MATCH_TEST_REGEX}\")
    endif()
    if(EXCLUDE_TEST_REGEX)
        set(EXCLUDE_TEST_ARGS -E \"${EXCLUDE_TEST_REGEX}\")
    endif()

    # coverage target
    add_custom_target(coverage
        COMMENT "Generating code coverage..."
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        # Cleanup code counters
        COMMAND "${PATH_LCOV}" --directory . --zerocounters --quiet

        # Run tests
        COMMAND "${CMAKE_CTEST_COMMAND}" --quiet ${MATCH_TEST_ARGS} ${EXCLUDE_TEST_ARGS}

        # Capture the counters
        COMMAND "${PATH_LCOV}"
            --directory .
            --rc lcov_branch_coverage=1
            --rc 'lcov_excl_line=assert'
            --capture --quiet
            --output-file "${COVERAGE_FILE_RAW}"
        # Remove coverage of tests, system headers, etc.
        COMMAND "${PATH_LCOV}"
            --remove "${COVERAGE_FILE_RAW}" '${CMAKE_SOURCE_DIR}/tests/*'
            --rc lcov_branch_coverage=1
            --quiet --output-file "${COVERAGE_FILE_CLEAN}"
        # Generate HTML report
        COMMAND "${PATH_GENHTML}"
            --branch-coverage --function-coverage --quiet --title "${PROJECT_NAME}"
            --legend --show-details --output-directory "${COVERAGE_DIR}"
            "${COVERAGE_FILE_CLEAN}"
        # Delete the counters
        COMMAND "${CMAKE_COMMAND}" -E remove
            ${COVERAGE_FILE_RAW} ${COVERAGE_FILE_CLEAN}
    )

    add_custom_command(TARGET coverage POST_BUILD
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
        COMMENT "To see the code coverage report, open ${COVERAGE_DIR}index.html"
        COMMAND ;
    )
endfunction()
