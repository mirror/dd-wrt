#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

#######################################################
### Dependencies                                    ###
#######################################################
find_package(Catch2 2.13.8 CONFIG)
if (Catch2_FOUND)
    include(${Catch2_DIR}/Catch.cmake)
else ()
    FetchContent_Declare(Catch2 URL https://github.com/catchorg/Catch2/archive/refs/tags/v2.13.8.zip)
    FetchContent_GetProperties(Catch2)
    if (NOT Catch2_POPULATED)
        FetchContent_Populate(Catch2)
        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        set(CATCH_USE_VALGRIND OFF) # "Perform SelfTests with Valgrind"
        set(CATCH_BUILD_EXAMPLES OFF) # "Build documentation examples"
        set(CATCH_BUILD_EXTRA_TESTS OFF) # "Build extra tests"
        set(CATCH_BUILD_STATIC_LIBRARY OFF) # "Builds static library from the main implementation. EXPERIMENTAL"
        set(CATCH_ENABLE_COVERAGE OFF) # "Generate coverage for codecov.io"
        set(CATCH_ENABLE_WERROR OFF) # "Enable all warnings as errors"
        set(CATCH_INSTALL_DOCS OFF) # "Install documentation alongside library"
        set(CATCH_INSTALL_HELPERS ON) # "Install contrib alongside library"
        add_subdirectory(${catch2_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/catch2)
        include(${catch2_SOURCE_DIR}/contrib/Catch.cmake)
    endif ()
endif ()

# Create a common catch main for all tests
add_library(catch_main catch_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)
target_compile_features(catch_main PUBLIC cxx_std_17)

#######################################################
### Unit Tests                                      ###
#######################################################
# Macro to create test targets and register with ctest
# First parameter: filename without extension
# Other parameters: libraries to link in this test (if empty, it links libcppm)
macro(add_small_test TEST_NAME)
    # Identify libraries to link with
    set(EXTRA_MACRO_ARGS ${ARGN})
    list(LENGTH EXTRA_MACRO_ARGS NUM_EXTRA_ARGS)
    if (${NUM_EXTRA_ARGS} GREATER 0)
        set(LINK_LIBS ${ARGN})
    else ()
        set(LINK_LIBS small::small)
    endif ()

    # Check if these libraries really exist
    set(LINK_LIBS_EXIST TRUE)
    foreach (LINK_LIB ${LINK_LIBS})
        if (NOT TARGET ${LINK_LIB})
            set(LINK_LIBS_EXIST FALSE)
            break()
        endif ()
    endforeach ()

    if (LINK_LIBS_EXIST)
        # Create executable for test
        add_executable(ut_${TEST_NAME} ${TEST_NAME}.cpp)

        # Link with catch-main
        target_link_libraries(ut_${TEST_NAME} PUBLIC ${LINK_LIBS} catch_main)

        # Enable UTF-8 on windows
        target_msvc_compile_options(ut_${TEST_NAME} INTERFACE "/utf-8")

        # Register with ctest
        catch_discover_tests(ut_${TEST_NAME})
    else ()
        # Library not found. Throw.
        message(FATAL_ERROR "${LINK_LIBS} does not exist")
    endif ()
endmacro()

# Main tests
add_small_test(ptr_wrapper small::small)
add_small_test(pod_small_vector small::small)
add_small_test(string_small_vector small::small)
add_small_test(custom_small_vector small::small)
add_small_test(shared_ptr_small_vector small::small)

add_small_test(unicode_functions small::small)
add_small_test(small_string_make small::small)
add_small_test(small_string_access small::small)
add_small_test(small_string_modify small::small)
add_small_test(small_string_modify_algorithms small::small)
add_small_test(small_string_const_algorithms small::small)
add_small_test(small_string_non_member small::small)

add_small_test(small_map small::small)
add_small_test(small_set small::small)
