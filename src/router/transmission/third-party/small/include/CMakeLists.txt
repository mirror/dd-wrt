#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

#######################################################
### Small containers library                        ###
#######################################################
# # CONFIGURE_DEPENDS works on most generators now
file(GLOB_RECURSE SMALL_HEADERS CONFIGURE_DEPENDS small/**/*.hpp)
add_library(small INTERFACE)
add_library(small::small ALIAS small)

# This library requires C++17
target_compile_features(small INTERFACE cxx_std_17)

# Include directories (development and installation)
target_include_directories(small ${warning_guard} INTERFACE
        $<BUILD_INTERFACE:${SMALL_ROOT_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Always enable warnings in development mode
maybe_target_pedantic_warnings(small)
