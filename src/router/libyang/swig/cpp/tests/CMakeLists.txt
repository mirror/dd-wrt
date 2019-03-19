include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_compile_options(-std=c++11)

set(CPP_TEST_SRC
  test_libyang
  test_tree_data
  test_tree_schema)

macro(ADD_CPP_TEST TEST)
  set(test_name cpp_${TEST})
  add_executable(${test_name} ${TEST}.cpp)
  target_link_libraries(${test_name} libyang-cpp)
  add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
  set_property(TEST ${test_name} PROPERTY ENVIRONMENT "LIBYANG_EXTENSIONS_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/extensions")
  set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT "MALLOC_CHECK_=3")
endmacro(ADD_CPP_TEST)

foreach(TEST ${CPP_TEST_SRC})
  ADD_CPP_TEST(${TEST})
endforeach()

if(ENABLE_STATIC)
    message(WARNING "Can't run C++ valgrind tests on a static build")
else()
    find_program(VALGRIND_FOUND valgrind)

    if(ENABLE_VALGRIND_TESTS)
        if(VALGRIND_FOUND)
            foreach(test_name IN LISTS CPP_TEST_SRC)
                add_test(cpp_${test_name}_valgrind valgrind --leak-check=full --show-leak-kinds=all --suppressions=${PROJECT_SOURCE_DIR}/../tests/ld.supp --error-exitcode=1
                     ${CMAKE_CURRENT_BINARY_DIR}/cpp_${test_name})
                 set_property(TEST cpp_${test_name}_valgrind PROPERTY ENVIRONMENT "LIBYANG_EXTENSIONS_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/extensions")
            endforeach(test_name)
        else(VALGRIND_FOUND)
            message(WARNING "valgrind executable not found! Disabling memory leaks tests.")
        endif(VALGRIND_FOUND)
    endif()
endif(ENABLE_STATIC)
