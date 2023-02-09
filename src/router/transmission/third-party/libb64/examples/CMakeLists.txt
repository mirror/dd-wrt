foreach(E c-example1 c-example2)
    add_executable(libb64-${E}
        ${E}.c)

    target_link_libraries(libb64-${E}
        PRIVATE
            libb64)
endforeach()

if(LIBB64_ENABLE_TESTS)
    add_test(
        NAME libb64-c-example1
        COMMAND libb64-c-example1)

    add_test(
        NAME libb64-c-example2
        COMMAND
            libb64-c-example2
                $<SHELL_PATH:${CMAKE_CURRENT_LIST_DIR}/loremgibson.txt>
                $<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/encoded.txt>
                $<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/decoded.txt>)

    add_test(
        NAME libb64-c-example2-compare
        COMMAND
            ${CMAKE_COMMAND} -E compare_files
                $<SHELL_PATH:${CMAKE_CURRENT_LIST_DIR}/loremgibson.txt>
                $<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/decoded.txt>)

    set_tests_properties(libb64-c-example2-compare
        PROPERTIES
            DEPENDS libb64-c-example2)
endif()
