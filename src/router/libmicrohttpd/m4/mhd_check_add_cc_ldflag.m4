# SYNOPSIS
#
#   MHD_CHECK_ADD_CC_LDFLAG([FLAG-TO-TEST], [VARIABLE-TO-EXTEND],
#                           [ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED])
#
# DESCRIPTION
#
#   This macro checks whether the specific compiler flag is supported.
#   The check is performing by appending FLAG-TO-TEST to the value of
#   VARIABLE-TO-EXTEND (LDFLAGS if not specified), then prepending result to
#   LDFLAGS (unless VARIABLE-TO-EXTEND is LDFLAGS), and then performing compile
#   and link test. If test succeed without warnings, then the flag is added to
#   VARIABLE-TO-EXTEND. Otherwise, if compile and link without test flag cannot
#   be done without any warning, the flag is considered to be unsuppoted.
#
#   Example usage:
#
#     MHD_CHECK_ADD_CC_LDFLAG([-pie], [additional_LDFLAGS])
#
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

AC_DEFUN([MHD_CHECK_ADD_CC_LDFLAG],[dnl
_MHD_CHECK_ADD_CC_XFLAG([$1],[$2],[$3],[$4],[[LDFLAGS]])dnl
])
