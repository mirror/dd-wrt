#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

###########################################################
### Linter application                                  ###
###########################################################
# This is a linter with a few rules that go beyond clang-format
add_executable(linter main.cpp)
target_compile_features(linter PUBLIC cxx_std_17)

###########################################################
### Lint library files                                  ###
###########################################################
# Generate a single-header version of small
add_custom_target(
        lint_small
        COMMAND linter
        # Files
        --include_paths "${SMALL_ROOT_DIR}/include"
        # Linter options
        --show_progress
        # CMake command options
        DEPENDS linter ${SMALL_HEADERS}
        COMMENT "Run linter"
        VERBATIM
)
add_dependencies(small lint_small)