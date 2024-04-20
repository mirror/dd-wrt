# yanglint

set(lintsrc
    main.c
    main_ni.c
    cmd.c
    cmd_add.c
    cmd_clear.c
    cmd_data.c
    cmd_list.c
    cmd_load.c
    cmd_print.c
    cmd_searchpath.c
    common.c
    completion.c
    configuration.c
    linenoise/linenoise.c)

set(format_sources
    ${format_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    PARENT_SCOPE)

add_executable(yanglint ${lintsrc} ${compatsrc})
target_link_libraries(yanglint yang)
install(TARGETS yanglint DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_SOURCE_DIR}/tools/lint/yanglint.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
target_include_directories(yanglint BEFORE PRIVATE ${PROJECT_BINARY_DIR})

#
# tests
#
function(add_yanglint_test)
    cmake_parse_arguments(ADDTEST "" "NAME;SCRIPT" "" ${ARGN})
    set(TEST_NAME yanglint_${ADDTEST_NAME})

    add_test(NAME ${TEST_NAME} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/${ADDTEST_SCRIPT})
    set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT "YANGLINT=${PROJECT_BINARY_DIR}/yanglint")
endfunction(add_yanglint_test)

if(ENABLE_BUILD_TESTS)
    # tests of non-interactive mode using shunit2
    find_program(PATH_SHUNIT NAMES shunit2)
    if(NOT PATH_SHUNIT)
        message(WARNING "'shunit2' not found! The yanglint(1) non-interactive tests will not be available.")
    else()
        add_yanglint_test(NAME ni_list SCRIPT shunit2/list.sh)
    endif()

    # tests of interactive mode using expect
    find_program(PATH_EXPECT NAMES expect)
    if(NOT PATH_EXPECT)
        message(WARNING "'expect' not found! The yanglint(1) interactive tests will not be available.")
    else()
#        add_yanglint_test(NAME in_list SCRIPT expect/list.exp)
    endif()
endif()
