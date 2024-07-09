#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

#######################################################
### Developer mode                                  ###
#######################################################
# Developer mode enables targets and code paths in the CMake scripts that are
# only relevant for the developer(s) of small.
# Targets necessary to build the project must be provided unconditionally, so
# consumers can trivially build and package the project
if (SMALL_MASTER_PROJECT)
    option(SMALL_DEVELOPER_MODE "Enable developer mode" OFF)
    option(BUILD_SHARED_LIBS "Build shared libs." OFF)
endif ()

if (NOT SMALL_DEVELOPER_MODE)
    return()
endif()

#######################################################
### What to build                                   ###
#######################################################
# C++ targets
option(SMALL_BUILD_TESTS "Build tests" ON)
option(SMALL_BUILD_EXAMPLES "Build examples" ON)

# Custom targets
option(SMALL_BUILD_DOCS "Build documentation" OFF)
option(SMALL_BUILD_COVERAGE_REPORT "Enable coverage support" OFF)
option(SMALL_BUILD_LINT "Enable linting" OFF)

#######################################################
### How to build                                    ###
#######################################################
option(SMALL_BUILD_WITH_PEDANTIC_WARNINGS  "Use pedantic warnings." ON)
option(SMALL_BUILD_WITH_SANITIZERS "Build with sanitizers." ${DEBUG_MODE})
option(SMALL_CATCH2_REPORTER "Reporter Catch2 should use when invoked from ctest." "console")

#######################################################
### How to build                                    ###
#######################################################
option(SMALL_BUILD_WITH_MSVC_HACKS "Accept utf-8 in MSVC by default." ON)
option(SMALL_BUILD_WITH_UTF8 "Accept utf-8 in MSVC by default." ON)

#######################################################
### Apply global developer options                  ###
#######################################################
# In development, we can set some options for all targets
if (SMALL_MASTER_PROJECT)
    message("Setting global options")

    # This whole project is for coverage
    if (SMALL_BUILD_COVERAGE_REPORT)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(CMAKE_C_FLAGS "${DCMAKE_C_FLAGS} --coverage")
    endif ()

    # Maybe add sanitizers to all targets
    if (SMALL_BUILD_WITH_SANITIZERS AND NOT EMSCRIPTEN)
        add_sanitizers()
    endif ()

    # Allow exceptions in MSVC
    if (MSVC AND SMALL_BUILD_WITH_EXCEPTIONS)
        add_compile_options(/EHsc)
    endif ()

    # Allow utf-8 in MSVC
    if (SMALL_BUILD_WITH_UTF8 AND MSVC)
        set(CMAKE_CXX_FLAGS "/utf-8")
    endif ()

    # MSVC hack to disable windows min/max
    # http://www.suodenjoki.dk/us/archive/2010/min-max.htm
    if (MSVC AND SMALL_BUILD_WITH_MSVC_HACKS)
        # Check for min in Windows.h
        # include(CheckSymbolExists)
        # check_symbol_exists(min "WinDef.h" HAVE_WINDOWS_MINMAX)
        # if (NOT HAVE_WINDOWS_MINMAX)
        #     check_symbol_exists(min "Windows.h" HAVE_WINDOWS_MINMAX)
        # endif ()
        # if (HAVE_WINDOWS_MINMAX)
        add_compile_definitions(NOMINMAX)
        # endif ()
    endif ()
endif ()