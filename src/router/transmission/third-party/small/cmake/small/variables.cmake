#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

#######################################################
### Environment                                     ###
#######################################################
include(cmake/functions/all.cmake)
set_master_project_booleans()
set_debug_booleans()
set_optimization_flags()
set_compiler_booleans()

#######################################################
### Installer                                       ###
#######################################################
# CMake dependencies for installer
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

#######################################################
### Enable Find*.cmake scripts                      ###
#######################################################
# Append ./cmake directory to our include paths for the find_package scripts
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#######################################################
### Enable FetchContent                             ###
#######################################################
# Functions to find or download packages if we can't find_package
include(FetchContent)

#######################################################
### Disable warnings in FetchContent                ###
#######################################################
# target_include_directories with the SYSTEM modifier will request the compiler
# to omit warnings from the provided paths, if the compiler supports that
# This is to provide a user experience similar to find_package when
# add_subdirectory or FetchContent is used to consume this project
set(warning_guard "")
if (NOT SMALL_MASTER_PROJECT)
    option(
            SMALL_INCLUDES_WITH_SYSTEM
            "Use SYSTEM modifier for small's includes, disabling warnings"
            ON
    )
    mark_as_advanced(SMALL_INCLUDES_WITH_SYSTEM)
    if (SMALL_INCLUDES_WITH_SYSTEM)
        set(warning_guard SYSTEM)
    endif ()
endif ()