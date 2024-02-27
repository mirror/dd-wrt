# SYNOPSIS
#
#   MHD_FIND_ADD_CC_CFLAG_IFELSE([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                                [VARIABLE-TO-EXTEND],
#                                [FLAG1-TO-TEST], [FLAG2-TO-TEST], ...)
#
# DESCRIPTION
#
#   This macro checks whether the specific compiler flags are supported.
#   The flags are checked one-by-one. The checking is stopped when the first
#   supported flag found.
#   The checks are performing by appending FLAGx-TO-TEST to the value of
#   VARIABLE-TO-EXTEND (CFLAGS if not specified), then prepending result to
#   CFLAGS (unless VARIABLE-TO-EXTEND is CFLAGS), and then performing compile
#   and link test. If test succeed without warnings, then the flag is added to
#   VARIABLE-TO-EXTEND and next flags are not checked. If compile-link cycle
#   cannot be performed without warning with all tested flags, no flag is
#   added to the VARIABLE-TO-EXTEND.
#   If any suitable flag is found, ACTION-IF-FOUND is executed otherwise
#   ACTION-IF-NOT-FOUND is executed. Found flag (if any) is available as
#   value of shell variable $mhd_cc_found_flag during action execution.
#
#   Example usage:
#
#     MHD_FIND_ADD_CC_CFLAG_IFELSE([AC_MSG_NOTICE([Enabled debug information])],
#                                  [],
#                                  [additional_CFLAGS],
#                                  [-ggdb3], [-g3], [-ggdb], [-g])
#
#   Note: Unlike others MHD_CHECK_*CC_CFLAG* macro, this macro uses another
#   order of parameters.
#
# LICENSE
#
#   Copyright (c) 2022 Karlson2k (Evgeny Grin) <k2k@narod.ru>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([MHD_FIND_ADD_CC_CFLAG_IFELSE],[dnl
_MHD_FIND_ADD_CC_XFLAG([[CFLAGS]],$@)])
