# SYNOPSIS
#
#   MHD_FIND_ADD_CC_CFLAG([VARIABLE-TO-EXTEND],
#                         [FLAG1-TO-TEST], [FLAG2-TO-TEST], ...)
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
#
#   Example usage:
#
#     MHD_CHECK_CC_CFLAG([additional_CFLAGS],
#                        [-ggdb3], [-g3], [-ggdb], [-g])
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

#serial 2

AC_DEFUN([MHD_FIND_ADD_CC_CFLAG],[dnl
_MHD_FIND_ADD_CC_XFLAG([[CFLAGS]],[],[],$@)])


# SYNOPSIS
#
#   _MHD_FIND_ADD_CC_XFLAG([CFLAGS|LDFLAGS],
#                          [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                          [VARIABLE-TO-EXTEND],
#                          [FLAG1-TO-TEST], [FLAG2-TO-TEST], ...)
#
AC_DEFUN([_MHD_FIND_ADD_CC_XFLAG],[dnl
  AC_PREREQ([2.64])dnl for m4_ifnblank
  AC_LANG_ASSERT([C])dnl
  m4_if(m4_eval([$# >= 5]), [0], [m4_fatal([$0: Macro must have at least five parameters])])dnl
  m4_ifblank([$5],[m4_fatal([$0: Fifth macro argument must not be empty])])dnl
  m4_ifnblank([$2$3],[m4_newline([m4_n([AS_UNSET([mhd_cc_found_flag])])])])dnl
  m4_bmatch(_mhd_norm_expd([$1]), [^\(CFLAGS\|LDFLAGS\)$],[],dnl
   [m4_fatal([$0: First macro argument must be either 'CFLAGS' or 'LDFLAGS; ']_mhd_norm_expd([$1])[' is not supported])])dnl
  m4_ifnblank([$4],[_MHD_FIND_ADD_CC_XFLAG_BODY(m4_ifnblank([$2$3],[mhd_cc_found_flag]),[$1],[$4],m4_shiftn([4],$@))],dnl
  [_MHD_FIND_ADD_CC_XFLAG_BODY(m4_ifnblank([$2$3],[mhd_cc_found_flag]),[$1],[$1],m4_shiftn([4],$@))])dnl
  m4_ifnblank([$2$3],[
    AS_IF([test -n "${mhd_cc_found_flag}"],[$2],[$3])
    AS_UNSET([mhd_cc_found_flag])
    ])dnl
  ])dnl
])


# SYNOPSIS
#
#   _MHD_FIND_ADD_CC_XFLAG_BODY([VAR-TO-SET],
#                               [CFLAGS|LDFLAGS],
#                               [VARIABLE-TO-EXTEND],
#                               [FLAG1-TO-TEST], [FLAG2-TO-TEST], ...)
#
m4_define([_MHD_FIND_ADD_CC_XFLAG_BODY],[dnl
m4_version_prereq([2.64])dnl for m4_ifnblank
m4_if([$#],[0],[m4_fatal([$0: no parameters])])dnl
m4_bmatch(_mhd_norm_expd([$2]),[^\(CFLAGS\|LDFLAGS\)$],[],dnl
[m4_fatal([$0: Second macro argument must be either 'CFLAGS' or 'LDFLAGS; ']_mhd_norm_expd([$2])[' is not supported])])dnl
m4_if([$#],[1],[m4_fatal([$0: not enough parameters])])dnl
m4_if([$#],[2],[m4_fatal([$0: not enough parameters])])dnl
m4_if([$#],[3],[m4_fatal([$0: not enough parameters])])dnl
m4_if([$#],[4],[m4_ifnblank([$4],[_MHD_CHECK_ADD_CC_XFLAG([$4],[$3],m4_ifnblank([$1],[$1="_mhd_norm_expd([$4])"]),[],[$2])])],
[m4_ifnblank([$4],[_MHD_CHECK_ADD_CC_XFLAG([$4],[$3],m4_ifnblank([$1],[$1="_mhd_norm_expd([$4])"]),[$0([$1],[$2],[$3],m4_shiftn([4],$@))],[$2])],
[$0([$1],[$2],[$3],m4_shiftn([4],$@))])])dnl
])
