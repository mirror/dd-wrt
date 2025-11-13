# SYNOPSIS
#
#   MHD_CHECK_FUNC_RUN(FUNCTION_NAME,
#                      [INCLUDES=AC_INCLUDES_DEFAULT], CHECK_CODE
#                      COMMAND_IF_CROSS_COMPILING,
#                      [ACTION_IF_AVAILABLE], [ACTION_IF_NOT_AVAILABLE],
#                      [ADDITIONAL_LIBS])
#
# DESCRIPTION
#
#   This macro checks for presence of specific function by including
#   specified headers, checking for declaration of the function and then
#   compiling and linking CHECK_CODE. If all previous steps succeed,
#   the binary is executed.
#   The CHECK_CODE is full body of the "main" function and must include
#   the final "return" keyword.
#   This checks the declaration, the presence in library and the functionality.
#   If function is available then macro HAVE_function_name (the name
#   of the function conveted to all uppercase characters) is defined
#   automatically.
#   By using definition from provided headers, this macro ensures that
#   correct calling convention is used for detection.
#   The compilation and linking is tried even if cross-compiling. If they
#   failed then the result is "no".
#   The COMMAND_IF_CROSS_COMPILING commands are executed if and only if
#   the compilation and linking succeed. These commands must set 'cacheVar'
#   variable to "yes", "no", "assuming yes" or "assuming no".
#   ACTION_IF_SUCCEED is executed if result is "yes" or "assuming yes".
#   ACTION_IF_FAILED is executed if result is "no" or "assuming no".
#
#   Example usage:
#
#     MHD_CHECK_FUNC_RUN([memmem],
#                        [[#include <string.h>]],
#                        [[const void *ptr = memmem("aa", 2, "a", 1);
#                          if (((void*)0) == ptr) return 1;
#                            return 0;]],
#                        [cacheVar="assuming yes"],
#                        [var_use_memmem='yes'], [var_use_memmem='no'])
#
#   The cache variable used in check so if any test will not work
#   correctly on some platform, user may simply fix it by giving cache
#   variable in configure parameters, for example:
#
#     ./configure mhd_cv_works_func_memmem=no
#
#   This simplifies building from source on exotic platforms as patching
#   of configure.ac is not required to change the results of the tests.
#
# LICENSE
#
#   Copyright (c) 2024 Karlson2k (Evgeny Grin) <k2k@narod.ru>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

dnl $1: FUNCTION_NAME
dnl $2: INCLUDES
dnl $3: CHECK_CODE
dnl $4: COMMAND_IF_CROSS_COMPILING
dnl $5: ACTION_IF_AVAILABLE
dnl $6: ACTION_IF_NOT_AVAILABLE
dnl $7: ADDITIONAL_LIBS
AC_DEFUN([MHD_CHECK_FUNC_RUN],[dnl
AC_PREREQ([2.64])dnl for AS_VAR_SET_IF, m4_ifblank, m4_ifnblank, m4_default_nblank
m4_newline([[# Expansion of $0 macro starts here]])
AC_LANG_ASSERT([C])dnl
m4_ifblank(m4_translit([$1],[()],[  ]), [m4_fatal([$0: First macro argument ("FUNCTION_NAME") must not be empty])])dnl
m4_ifblank([$3], [m4_fatal([$0: Third macro argument ("CHECK_CODE") must not be empty])])dnl
m4_bmatch(m4_normalize([$1]), [\s],dnl
          [m4_fatal([$0: First macro argument ("FUNCTION_NAME") must not contain whitespaces])])dnl
m4_if(m4_index([$3], m4_normalize(m4_translit([$1],[()],[  ]))), [-1], dnl
      [m4_fatal([$0: "CHECK_CODE" macro parameter (third argument) does not contain ']m4_normalize([$1])[' token])])dnl
m4_if(m4_index([$3],return), [-1], dnl
      [m4_fatal([$0: "CHECK_CODE" macro parameter (third argument) does not contain 'return' token])])dnl
m4_bmatch(_mhd_norm_expd([$4]),[\<]cacheVar[\>],[],dnl
[m4_fatal([$0: The fourth macro argument ("COMMAND_IF_CROSS_COMPILING") must assign ]dnl
[a value to the cache variable 'cacheVar'. The variable must be not overquoted.])])dnl
AS_VAR_PUSHDEF([decl_cv_Var],[ac_cv_have_decl_]m4_bpatsubst(_mhd_norm_expd(m4_translit([$1],[()],[  ])),[[^a-zA-Z0-9]],[_]))dnl
AS_VAR_PUSHDEF([cacheVar],[mhd_cv_works_func_]m4_bpatsubst(_mhd_norm_expd(m4_translit([$1],[()],[  ])),[[^a-zA-Z0-9]],[_]))dnl
AS_VAR_SET_IF([cacheVar],[],[AC_CHECK_DECL(_mhd_norm_expd([$1]),[],[cacheVar="no"],[$2])])
AC_CACHE_CHECK([whether '$1' available and works],[cacheVar],dnl
[
AC_LANG_CONFTEST([AC_LANG_SOURCE([
m4_default_nblank([$2],[AC_INCLUDES_DEFAULT])

[int main(void)
{
  ]$3[
}
]])])
m4_ifnblank([$7],[dnl
mhd_check_func_SAVE_LIBS="$LIBS"
LIBS="_mhd_norm_expd([$7]) $LIBS"
])dnl m4_ifnblank
AS_VAR_IF([cross_compiling],["yes"],
[AC_LINK_IFELSE([],[
$4
],[cacheVar='no'])dnl AC_LINK_IFELSE
],dnl
[AC_RUN_IFELSE([],[cacheVar='yes'],[cacheVar='no'],[[# Dummy placeholder]])
])
rm -f conftest.$ac_ext
m4_ifnblank([$7],[dnl
  LIBS="${mhd_check_func_SAVE_LIBS}"
  AS_UNSET([mhd_check_func_SAVE_LIBS])
])dnl m4_ifnblank
])dnl AC_CACHE_CHECK
m4_ifnblank([$5],[
AS_IF([test "x$cacheVar" = "xyes" || test "x$cacheVar" = "xassuming yes"],[$5])dnl AS_IF
])dnl m4_ifnblank
m4_ifnblank([$6],[
AS_IF([test "x$cacheVar" = "xno" || test "x$cacheVar" = "xassuming no"],[$6])dnl AS_IF
])dnl m4_ifnblank
m4_newline([[# Expansion of $0 macro ends here]])
])dnl AC_DEFUN MHD_CHECK_FUNC_RUN
