#!/bin/sh
#
# This script helps with the libConfuse coding style, but
# remember: "indent" is not a fix for bad programming.
# 
# Rules:
#   - Linux style, https://www.kernel.org/doc/Documentation/process/coding-style.rst
#   - Don't touch comment content/formatting, only placement.
#   - ignore any ~/.indent.pro
#
# With the -T <type> we can inform indent about non-ansi types that
# we've added, so indent doesn't insert spaces in odd places.
# 
indent --linux-style							\
       --ignore-profile							\
       --preserve-mtime							\
       --break-after-boolean-operator					\
       --blank-lines-after-procedures					\
       --blank-lines-after-declarations					\
       --dont-break-function-decl-args					\
       --dont-break-procedure-type					\
       --leave-preprocessor-space					\
       --line-length132							\
       --honour-newlines						\
       --space-after-if							\
       --space-after-for						\
       --space-after-while						\
       --leave-optional-blank-lines					\
       --dont-format-comments						\
       --no-blank-lines-after-commas					\
       --no-space-after-parentheses					\
       --no-space-after-casts						\
       -T size_t -T FILE -T uint32_t -T uint16_t -T uint8_t		\
       -T cfg_type_t -T cfg_value_t -T cfg_simple_t -T cfg_opt_t	\
       -T cfg_t -T cfg_defvalue_t -T cfg_flag_t -T cfg_searchpath_t	\
       -T cfg_bool_t -T cfg_func_t -T cfg_free_func_t -T cfg_errfunc_t	\
       -T cfg_print_func_t -T cfg_callback_t -T cfg_validate_callback_t	\
$*
