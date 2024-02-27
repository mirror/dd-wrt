# SYNOPSIS
#
#   MHD_CHECK_ADD_CC_LDFLAGS([FLAGS-TO-TEST], [VARIABLE-TO-EXTEND])
#
# DESCRIPTION
#
#   This macro checks whether the specific compiler flags are supported.
#   The FLAGS-TO-TEST parameter is whitespace-separated flagto to test.
#   The flags are tested one-by-one, all supported flags are added to the
#   VARIABLE-TO-EXTEND.
#   Every flag check is performing by appending one flag to the value of
#   VARIABLE-TO-EXTEND (LDFLAGS if not specified), then prepending result to
#   LDFLAGS (unless VARIABLE-TO-EXTEND is LDFLAGS), and then performing compile
#   and link test. If test succeed without warnings, then the flag is added to
#   VARIABLE-TO-EXTEND. Otherwise, if compile and link without test flag cannot
#   be done without any warning, the flag is considered to be unsuppoted.
#
#   Example usage:
#
#     MHD_CHECK_ADD_CC_LDFLAGS([-W,--strip-all -Wl,--fatal-warnings],
#                              [additional_LDFLAGS])
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

AC_DEFUN([MHD_CHECK_ADD_CC_LDFLAGS],[dnl
m4_foreach_w([test_flag],[$1],
[MHD_CHECK_ADD_CC_LDFLAG([test_flag],[$2])
])dnl
])
