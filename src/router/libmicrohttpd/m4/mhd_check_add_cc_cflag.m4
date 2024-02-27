# SYNOPSIS
#
#   MHD_CHECK_ADD_CC_CFLAG([FLAG-TO-TEST], [VARIABLE-TO-EXTEND],
#                          [ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED])
#
# DESCRIPTION
#
#   This macro checks whether the specific compiler flag is supported.
#   The check is performing by appending FLAG-TO-TEST to the value of
#   VARIABLE-TO-EXTEND (CFLAGS if not specified), then prepending result to
#   CFLAGS (unless VARIABLE-TO-EXTEND is CFLAGS), and then performing compile
#   and link test. If test succeed without warnings, then the flag is added to
#   VARIABLE-TO-EXTEND. Otherwise, if compile and link without test flag cannot
#   be done without any warning, the flag is considered to be unsuppoted.
#
#   Example usage:
#
#     MHD_CHECK_ADD_CC_CFLAG([-Wshadow], [additional_CFLAGS])
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

AC_DEFUN([MHD_CHECK_ADD_CC_CFLAG],[dnl
_MHD_CHECK_ADD_CC_XFLAG([$1],[$2],[$3],[$4],[[CFLAGS]])dnl
])


# SYNOPSIS
#
#   _MHD_CHECK_ADD_CC_XFLAG([FLAG-TO-TEST], [VARIABLE-TO-EXTEND],
#                           [ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED],
#                           [CFLAGS|LDFLAGS])
#
AC_DEFUN([_MHD_CHECK_ADD_CC_XFLAG],[dnl
  AC_PREREQ([2.64])dnl for AS_VAR_IF, m4_ifnblank, m4_default
  AC_LANG_ASSERT([C])dnl
  m4_ifblank([$1], [m4_fatal([First macro argument must not be empty])])dnl
  m4_bmatch(_mhd_norm_expd([$1]), [\s],dnl
            [m4_fatal([First macro argument must not contain whitespaces])])dnl
  m4_bmatch(_mhd_norm_expd([$2]), [\s],dnl
            [m4_fatal([Second macro argument must not contain whitespaces])])dnl
  m4_bmatch([$1], [\$], [m4_fatal([$0: First macro argument must not contain '$'])])dnl
  m4_bmatch([$2], [\$], [m4_fatal([$0: Second macro argument must not contain '$'])])dnl
  m4_bmatch(_mhd_norm_expd([$5]), [^\(CFLAGS\|LDFLAGS\)$],[],dnl
   [m4_fatal([$0: Fifth macro argument must be either 'CFLAGS' or 'LDFLAGS; ']_mhd_norm_expd([$5])[' is not supported])])dnl
  m4_ifnblank([$2],
    [_MHD_CHECK_CC_XFLAG([$1], [$2],
      [MHD_APPEND_FLAG_TO_VAR(_mhd_norm_expd([$2]),_mhd_norm_expd([$1]))
       $3],[$4],[$5])],
    [_MHD_CHECK_CC_XFLAG([$1],_mhd_norm_expd([$5]),
      [MHD_APPEND_FLAG_TO_VAR(_mhd_norm_expd([$5]),_mhd_norm_expd([$1]))
       $3],[$4],[$5])]
  )
])
