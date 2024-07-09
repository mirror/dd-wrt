#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

#######################################################
### Small                                           ###
#######################################################
# Project information
cmake_minimum_required(VERSION 3.15)
project(
        small
        VERSION 0.2.1
        DESCRIPTION "C++ small containers"
        HOMEPAGE_URL "https://alandefreitas.github.io/small"
)

set(SMALL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SMALL_VERSION ${CMAKE_PROJECT_VERSION})
message(STATUS "${CMAKE_PROJECT_NAME} v${CMAKE_PROJECT_VERSION}: ${CMAKE_PROJECT_HOMEPAGE_URL}")

include(cmake/small/in_source_guard.cmake)
include(cmake/small/variables.cmake)

#######################################################
### Options                                         ###
#######################################################
# What to build
option(SMALL_BUILD_INSTALLER "Build installer target" ${SMALL_MASTER_PROJECT})
option(SMALL_BUILD_PACKAGE "Build package" ${SMALL_MASTER_PROJECT})

# How to build
option(SMALL_BUILD_WITH_EXCEPTIONS "Add compiler flags to use exceptions." ON)

# Development options
include(cmake/small/dev-options.cmake)

#######################################################
### Libraries                                       ###
#######################################################
add_subdirectory(include)

#######################################################
### Installer                                       ###
#######################################################
if (SMALL_BUILD_INSTALLER)
    # Install targets
    install(TARGETS small
            EXPORT small-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )

    # Install headers
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/small
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.hpp"
            )

    # Install cmake targets script
    install(EXPORT small-targets
            FILE small-targets.cmake
            NAMESPACE small::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/small
            )

    # https://cliutils.gitlab.io/modern-cmake/chapters/install/installing.html
    # Set variable where the cmake config is
    set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/small)
    message(STATUS "${CMAKE_PROJECT_NAME} install directory: ${CMAKE_INSTALL_PREFIX}")
    message(STATUS "${CMAKE_PROJECT_NAME} library install directory: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

    # Create small-config-version.cmake and install it
    if (CMAKE_PROJECT_VERSION VERSION_LESS 1.0.0)
        set(compatibility_mode SameMajorVersion)
    else ()
        set(compatibility_mode SameMinorVersion)
    endif ()
    write_basic_package_version_file(
            small-config-version.cmake
            VERSION ${PACKAGE_VERSION}
            COMPATIBILITY ${compatibility_mode}
    )

    # Install the file small-config-version.cmake
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/small-config-version.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/small)

    # Create small-config.cmake from small-config.cmake.in
    configure_package_config_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/small-config.cmake.in # input file
            ${CMAKE_CURRENT_BINARY_DIR}/small-config.cmake    # output file
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/small
    )

    # Install the file small-config.cmake
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/small-config.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/small)
endif ()

#######################################################
### Packages                                        ###
#######################################################
if (SMALL_BUILD_INSTALLER AND SMALL_BUILD_PACKAGE)
    # Set the cpack variables
    # https://cliutils.gitlab.io/modern-cmake/chapters/install/packaging.html

    # The most common cpack variables
    set(CPACK_PACKAGE_VENDOR "small")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "small: C++ small containers")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_RESOURCE_FILE_LICENSE "${SMALL_ROOT_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${SMALL_ROOT_DIR}/README.md")

    # Set CPACK_SOURCE_IGNORE_FILES with files source packages shouldn't install
    # We get these from .gitignore to avoid redundancy
    FILE(READ .gitignore GITIGNORE_CONTENTS)
    STRING(REGEX REPLACE ";" "\\\\;" GITIGNORE_CONTENTS "${GITIGNORE_CONTENTS}")
    STRING(REGEX REPLACE "\n" ";" GITIGNORE_CONTENTS "${GITIGNORE_CONTENTS}")
    set(CPACK_SOURCE_IGNORE_FILES ${GITIGNORE_CONTENTS})

    # Always include CPack at last
    include(CPack)
endif ()

#######################################################
### Developer mode                                  ###
#######################################################
if (NOT SMALL_DEVELOPER_MODE)
    return()
elseif (NOT SMALL_MASTER_PROJECT)
    message(
            AUTHOR_WARNING
            "Developer mode is intended for developers"
    )
endif ()

add_subdirectory(dev-tools)

if (SMALL_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif ()

if (SMALL_BUILD_DOCS)
    add_subdirectory(docs)
endif ()

if (SMALL_BUILD_COVERAGE_REPORT)
    include(cmake/small/coverage-report.cmake)
endif ()

if (SMALL_BUILD_LINT)
    include(cmake/small/lint-targets.cmake)
endif ()

#######################################################
### Examples                                        ###
#######################################################
if (SMALL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
