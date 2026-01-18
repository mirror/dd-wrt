#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#


# @brief Enable pedantic warnings for a target
# - This does not propagate to other targets
function(target_pedantic_warnings TARGET_NAME)
    # Set warning levels to about the same level for MSVC, GCC, and Clang
    if (MSVC)
        target_compile_options(${TARGET_NAME} INTERFACE /W4 /WX)
    else ()
        target_compile_options(${TARGET_NAME} INTERFACE -Wall -Wextra -pedantic -Werror)
    endif ()
endfunction()

# @brief Maybe enable pedantic warnings for a target
function(maybe_target_pedantic_warnings TARGET_NAME)
    if (SMALL_BUILD_WITH_PEDANTIC_WARNINGS)
        target_pedantic_warnings(${TARGET_NAME})
    endif ()
endfunction ()

# @brief Sets pedantic compiler options for all targets
# - In a more serious project, we would do that per target
# - Setting it for all targets unfortunately affects
# external libraries, which often lead to some warnings.
macro(add_pedantic_warnings)
    if (MSVC)
        add_compile_options(/W4 /WX)
    else ()
        add_compile_options(-Wall -Wextra -pedantic -Werror)
    endif ()
endmacro()

# @brief Maybe set pedantic compiler options for all targets
macro(maybe_add_pedantic_warnings)
    if (SMALL_BUILD_WITH_PEDANTIC_WARNINGS)
        add_pedantic_warnings()
    endif ()
endmacro()
