add_executable(quickstart-example main.cpp)
target_link_libraries(quickstart-example small)

add_executable(default_inline_storage-example default_inline_storage.cpp)
target_link_libraries(default_inline_storage-example small)

add_executable(unicode_strings-example unicode_strings.cpp)
target_link_libraries(unicode_strings-example small)

add_executable(associative-example associative.cpp)
target_link_libraries(associative-example small)

if (SMALL_BUILD_TESTS)
    add_test(NAME quickstart-example-as-test
            COMMAND quickstart-example)
    add_test(NAME default_inline_storage-example-as-test
            COMMAND default_inline_storage-example)
    add_test(NAME unicode_strings-example-as-test
            COMMAND unicode_strings-example)
    add_test(NAME associative-example-as-test
            COMMAND associative-example)
endif ()