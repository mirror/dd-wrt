#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

# @brief Set variable indicating if this is a master project
# - This is important to avoid building tests and examples when project is not master
macro(set_master_project_booleans)
    if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
        set(SMALL_MASTER_PROJECT ON)
    else ()
        set(SMALL_MASTER_PROJECT OFF)
    endif ()
endmacro()

# @brief Set variables indicating if mode is Debug or Release
# - The mode might be neither of them
macro(set_debug_booleans)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(DEBUG_MODE ON)
        set(NOT_DEBUG_MODE OFF)
        set(RELEASE_MODE OFF)
        set(NOT_RELEASE_MODE ON)
    else ()
        set(DEBUG_MODE OFF)
        set(NOT_DEBUG_MODE ON)
        set(RELEASE_MODE ON)
        set(NOT_RELEASE_MODE OFF)
    endif ()
endmacro()

# @brief Create booleans GCC and CLANG to identify the compiler more easily
# - A boolean for MSVC already exists by default
macro(set_compiler_booleans)
    set(CLANG OFF)
    set(GCC OFF)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(CLANG ON)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(GCC ON)
    endif ()

    # Check if we are using the "expected" compilers, which are usually
    # Win+MSVC, Linux+GCC, Mac+Clang
    set(EXPECTED_COMPILER OFF)
    if (WIN32 AND MSVC)
        set(EXPECTED_COMPILER ON)
    elseif (APPLE AND CLANG)
        set(EXPECTED_COMPILER ON)
    elseif (UNIX AND NOT APPLE AND GCC)
        set(EXPECTED_COMPILER ON)
    endif()
endmacro()
