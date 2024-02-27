# SYNOPSIS
#
#   MHD_PREPEND_FLAG_TO_VAR([VARIABLE-TO-EXTEND], [FLAG-TO-PREPEND])
#
# DESCRIPTION
#
#   This macro sets VARIABLE-TO-EXTEND to the value of VARIABLE-TO-EXTEND with
#   appended FLAG-TO-APPEND. If current value of VARIABLE-TO-EXTEND and
#   FLAG-TO-APPEND are both non-empty strings then space is added between them.
#
#   Example usage:
#
#     MHD_PREPEND_FLAG_TO_VAR([my_CFLAGS], [-Wall])
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

#serial 2

AC_DEFUN([MHD_PREPEND_FLAG_TO_VAR],[dnl
m4_ifblank([$1],[m4_fatal([$0: First macro argument must not be empty])])dnl
m4_bmatch([$1], [\$], [m4_fatal([$0: First macro argument must not contain '$'])])dnl
m4_bmatch([$1], [,], [m4_fatal([$0: First macro argument must not contain ','])])dnl
m4_bmatch(_mhd_norm_expd([$1]), [\s],dnl
[m4_fatal([$0: First macro argument must not contain whitespaces])])dnl
m4_pushdef([varExtd],_mhd_norm_expd([$1]))dnl
m4_bmatch([$2],[\$],dnl
[dnl The second parameter is a variable value
AS_IF([test -z "_mhd_norm_expd([$2])"],dnl
[varExtd="${varExtd}"],dnl
[test -z "${varExtd}"],dnl
[varExtd="_mhd_norm_expd([$2])"],dnl
[varExtd="_mhd_norm_expd([$2]) ${varExtd}"])
],dnl
[dnl The second parameter is not a variable value
m4_ifnblank(_mhd_norm_expd([$2]),dnl
[AS_IF([test -z "${varExtd}"],[varExtd="_mhd_norm_expd([$2])"],[varExtd="_mhd_norm_expd([$2]) ${varExtd}"])
],dnl
[m4_n([varExtd="${varExtd}"])])])dnl m4_ifnblank m4_bmatch
m4_popdef([varExtd])dnl
])dnl AC_DEFUN
