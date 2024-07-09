#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

#######################################################
### Project properties                              ###
#######################################################
cmake_minimum_required(VERSION 3.15)

project(cmake_subdir_test LANGUAGES CXX)

#######################################################
### Integration                                     ###
#######################################################
if (BOOST_CI_INSTALL_TEST)
    find_package(small REQUIRED)
else ()
    set(SMALL_DIR ../..)
    set(SMALL_BINARY_DIR alandefreitas/small)
    add_subdirectory(${SMALL_DIR} ${SMALL_BINARY_DIR})
endif ()

#######################################################
### Print small target properties                   ###
#######################################################
# This is useful output for CI
if(NOT CMAKE_PROPERTY_LIST)
    execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)

    # Convert command output into a CMake list
    string(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
    string(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
endif()

function(print_properties)
    message("CMAKE_PROPERTY_LIST = ${CMAKE_PROPERTY_LIST}")
endfunction()

function(print_target_properties target)
    if(NOT TARGET ${target})
        message(STATUS "There is no target named '${target}'")
        return()
    endif()

    foreach(property ${CMAKE_PROPERTY_LIST})
        string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" property ${property})

        # Fix https://stackoverflow.com/questions/32197663/how-can-i-remove-the-the-location-property-may-not-be-read-from-target-error-i
        if(property STREQUAL "LOCATION" OR property MATCHES "^LOCATION_" OR property MATCHES "_LOCATION$")
            continue()
        endif()

        get_property(was_set TARGET ${target} PROPERTY ${property} SET)
        if(was_set)
            get_target_property(value ${target} ${property})
            message("${target} ${property} = ${value}")
        endif()
    endforeach()
endfunction()

print_target_properties(small::small)

#######################################################
### Target                                          ###
#######################################################
add_executable(main main.cpp)
target_link_libraries(main small::small)
if (MSVC)
    target_compile_options(main PRIVATE /EHsc)
endif()

#######################################################
### Tests                                           ###
#######################################################
enable_testing()
add_test(NAME main COMMAND main)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>)