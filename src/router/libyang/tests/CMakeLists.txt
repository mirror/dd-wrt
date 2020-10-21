cmake_minimum_required(VERSION 2.8.12)

# Correct RPATH usage on OS X
set(CMAKE_MACOSX_RPATH TRUE)

# Set TESTS_DIR to realpath
get_filename_component(TESTS_DIR "${CMAKE_SOURCE_DIR}/tests" REALPATH)

set(api_tests test_libyang test_tree_schema test_xml test_dict test_tree_data test_tree_data_dup test_tree_data_merge test_xpath test_xpath_1.1 test_diff)
set(data_tests test_data_initialization test_leafref_remove test_instid_remove test_keys test_autodel test_when test_when_1.1 test_must_1.1 test_defaults test_emptycont test_unique test_mandatory test_json test_parse_print test_values test_metadata test_yangtypes_xpath test_yang_data test_yang_data_ns test_unknown_element test_user_types)
set(schema_yin_tests test_print_transform)
set(schema_tests test_ietf test_augment test_deviation test_refine test_typedef test_import test_include test_feature test_conformance test_leaflist test_status test_printer test_invalid)
if(CMAKE_BUILD_TYPE MATCHES debug)
    list(APPEND schema_tests test_extensions)
endif(CMAKE_BUILD_TYPE MATCHES debug)
set(conformance_tests test_sec6_1_1 test_sec6_2 test_sec5_1 test_sec5_5 test_sec6_1_3 test_sec6_2_1 test_sec7_1 test_sec7_2 test_sec7_3 test_sec7_3_1 test_sec7_3_4 test_sec7_5_2 test_sec7_5_4 test_sec7_5_5 test_sec7_6_2 test_sec7_6_3 test_sec7_6_4 test_sec7_6_5 test_sec7_7_2 test_sec7_7_3 test_sec7_7_4 test_sec7_7_5 test_sec7_8_1 test_sec7_8_2 test_sec7_8_3 test_sec7_9_1 test_sec7_9_2 test_sec7_9_3 test_sec7_9_4 test_sec7_10 test_sec7_11 test_sec7_12_1 test_sec7_12_2 test_sec7_13_1 test_sec7_13_2 test_sec7_13_3 test_sec7_14 test_sec7_15 test_sec7_15_1 test_sec7_16_1 test_sec7_16_2 test_sec7_18_1 test_sec7_18_2 test_sec7_18_3_1 test_sec7_18_3_2 test_sec7_19_1 test_sec7_19_2 test_sec7_19_5 test_sec9_2 test_sec9_3 test_sec9_4_4 test_sec9_4_6 test_sec9_5 test_sec9_6 test_sec9_7 test_sec9_8 test_sec9_9 test_sec9_10 test_sec9_11 test_sec9_12 test_sec9_13)
set(internal_tests test_lyb test_hash_table test_state_lists)

include_directories(SYSTEM ${CMOCKA_INCLUDE_DIR})

foreach(test_name IN LISTS api_tests)
    add_executable(${test_name} api/${test_name}.c)
endforeach(test_name)

foreach(test_name IN LISTS data_tests)
    add_executable(${test_name} data/${test_name}.c)
endforeach(test_name)

foreach(test_name IN LISTS schema_yin_tests)
    add_executable(${test_name} schema/yin/${test_name}.c)
endforeach(test_name)

foreach(test_name IN LISTS schema_tests)
    add_executable(${test_name} schema/${test_name}.c)
endforeach(test_name)

foreach(test_name IN LISTS conformance_tests)
    add_executable(${test_name} conformance/${test_name}.c)
endforeach(test_name)

foreach(test_name IN LISTS internal_tests)
    add_executable(${test_name} internal/${test_name}.c $<TARGET_OBJECTS:yangobj_tests>)
endforeach(test_name)

# Set common attributes of all tests
foreach(test_name IN LISTS api_tests data_tests schema_yin_tests schema_tests conformance_tests internal_tests)
    target_link_libraries(${test_name} ${CMOCKA_LIBRARIES} yang)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
    set_property(TEST ${test_name} PROPERTY ENVIRONMENT "LIBYANG_EXTENSIONS_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/extensions")
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT "LIBYANG_USER_TYPES_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/user_types")
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT "MALLOC_CHECK_=3")
endforeach(test_name)

configure_file("${PROJECT_SOURCE_DIR}/tests/config.h.in" "${PROJECT_BINARY_DIR}/tests/config.h" ESCAPE_QUOTES @ONLY)
include_directories(${PROJECT_BINARY_DIR})

find_program(VALGRIND_FOUND valgrind)
if(ENABLE_VALGRIND_TESTS)
    if(VALGRIND_FOUND)
        foreach(test_name IN LISTS api_tests data_tests schema_yin_tests schema_tests conformance_tests internal_tests)
            add_test(${test_name}_valgrind valgrind --leak-check=full --show-leak-kinds=all --suppressions=${PROJECT_SOURCE_DIR}/tests/ld.supp --error-exitcode=1
                ${CMAKE_BINARY_DIR}/tests/${test_name})
            set_property(TEST ${test_name}_valgrind PROPERTY ENVIRONMENT "LIBYANG_EXTENSIONS_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/extensions")
            set_property(TEST ${test_name}_valgrind APPEND PROPERTY ENVIRONMENT "LIBYANG_USER_TYPES_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/user_types")
        endforeach(test_name)
    else(VALGRIND_FOUND)
        message(WARNING "valgrind executable not found! Disabling memory leaks tests.")
    endif(VALGRIND_FOUND)
endif(ENABLE_VALGRIND_TESTS)

if(ENABLE_CALLGRIND_TESTS)
    string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
    if(NOT (BUILD_TYPE STREQUAL "release"))
        message(WARNING "Not a release build type! Callgrind results may be inaccurate.")
    endif()

    if(VALGRIND_FOUND)
        find_path(VALGRIND_INCLUDE_DIRS NAMES valgrind/callgrind.h PATHS /usr/include /usr/local/include)
        if(VALGRIND_INCLUDE_DIRS)
            add_subdirectory(callgrind)
        else(VALGRIND_INCLUDE_DIRS)
            message(WARNING "valgrind include files not found! Disabling callgrind tests.")
        endif(VALGRIND_INCLUDE_DIRS)
    else(VALGRIND_FOUND)
        message(WARNING "valgrind executable not found! Disabling callgrind tests.")
    endif(VALGRIND_FOUND)
endif(ENABLE_CALLGRIND_TESTS)
