# SYNOPSIS
#
#   MHD_CHECK_LINK_RUN(MESSAGE, CACHE_ID, COMMAND_IF_CROSS_COMPILING, INPUT,
#                      [ACTION_IF_SUCCEED], [ACTION_IF_FAILED])
#
# DESCRIPTION
#
#   Improved version of AC_RUN_IFELSE macro.
#   Unlike AC_RUN_IFELSE, this macro tries to link the code if cross-compiling.
#   Action COMMAND_IF_CROSS_COMPILING is executed only if link is succeed,
#   otherwise CACHE_ID variable set to "no". 
#   COMMAND_IF_CROSS_COMPILING action must set CACHE_ID variable to "yes", "no",
#   "assuming yes" or "assuming no".
#   ACTION_IF_SUCCEED is executed if result is "yes" or "assuming yes".
#   ACTION_IF_FAILED is executed if result is "no" or "assuming no".
#
#   Example usage:
#
#     MHD_CHECK_LINK_RUN([for valid snprintf()], [mhd_cv_snprintf_valid],
#                        [AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT],
#                                         [if (4 != snprintf(NULL, 0, "test"))
#                                            return 2;])],
#                        [mhd_cv_snprintf_valid='assuming no'])
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

AC_DEFUN([MHD_CHECK_LINK_RUN],[dnl
m4_ifblank([$1],[m4_fatal([$0: The first macro argument ("MESSAGE") must not be empty])])dnl
m4_ifblank([$2],[m4_fatal([$0: The second macro argument ("CACHE_ID") must not be empty])])dnl
m4_ifblank([$3],[m4_fatal([$0: The third macro argument ("COMMAND_IF_CROSS_COMPILING") ]dnl
[must not be empty])])dnl
m4_ifblank([$4],[m4_fatal([$0: The fourth macro argument ("INPUT") must not be empty])])dnl
m4_bmatch(_mhd_norm_expd([$2]),[\s],dnl
[m4_fatal([$0: The second macro argument ("CACHE_ID") must not contain whitespaces])])dnl
m4_bmatch(_mhd_norm_expd([$3]),[\<]m4_re_escape(_mhd_norm_expd([$2]))[\>],[],dnl
[m4_fatal([$0: The third macro argument ("COMMAND_IF_CROSS_COMPILING") must assign ]dnl
[a value to the cache variable ']_mhd_norm_expd([$2])['])])dnl
m4_pushdef([cacheVar],_mhd_norm_expd([$2]))dnl
AC_CACHE_CHECK([$1],[$2],
[
AC_LANG_CONFTEST([$4])
AS_VAR_IF([cross_compiling],["yes"],
[AC_LINK_IFELSE([],[
$3
],[cacheVar='no'])dnl AC_LINK_IFELSE
],dnl
[AC_RUN_IFELSE([],[cacheVar='yes'],[cacheVar='no'],[[# Dummy placeholder]])
])
rm -f conftest.$ac_ext
])
m4_ifnblank([$5],[
AS_IF([test "x$cacheVar" = "xyes" || test "x$cacheVar" = "xassuming yes"],[$5])dnl AS_IF
])dnl m4_ifnblank
m4_ifnblank([$6],[
AS_IF([test "x$cacheVar" = "xno" || test "x$cacheVar" = "xassuming no"],[$6])dnl AS_IF
])dnl m4_ifnblank
])dnl AC_DEFUN
