# SYNOPSIS
#
#   MHD_BOOL
#
# DESCRIPTION
#
#   This macro checks whether the boolean keywords "bool", "true", "false"
#   are supported by compiler. If they are not supported, suitable replacement
#   macros are defined.
#
#   Example usage:
#
#     MHD_BOOL
#
#   The cache variables are used in the checks so if any test will not work
#   correctly on some platform, user may simply fix it by giving cache
#   variable in configure parameters, for example:
#
#     ./configure mhd_cv_bool_native=no
#
#   This simplify building from source on exotic platforms as patching
#   of configure.ac is not required to change results of tests.
#
# LICENSE
#
#   Copyright (c) 2024 Karlson2k (Evgeny Grin) <k2k@narod.ru>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 2

AC_DEFUN([MHD_BOOL],[dnl
AC_PREREQ([2.64])dnl for AS_VAR_IF
m4_newline([[# Expansion of macro $0 starts here]])
AC_LANG_ASSERT([C])dnl
AC_REQUIRE([AC_PROG_CC])dnl
AH_TEMPLATE([[HAVE_STDBOOL_H]], [Define to 1 i][f you have the <stdbool.h> header file and <stdbool.h> is required to use 'bool' type.])dnl
AH_TEMPLATE([[HAVE_BUILTIN_TYPE_BOOL]], [Define to 1 i][f you have the boolean type that takes only 'true' and 'false'.])dnl
AH_TEMPLATE([[bool]], [Define to the name of the type which is used as a replacemnt f][or boolean type.])dnl
[mhd_bool_test_code='
static bool get_false(void)
{
  static bool test_arr[sizeof(bool)*2 - 1] = {false};
  return test_arr[0];
}

int main(void)
{
  static bool var1 = true;
  static bool var2 = false;
  static int int_arr[2 - 3 * ((int) (false))] = {(int) true, (int) false};
  static bool bool_arr[2 - 3 * ((int) (!true))] = {false, true};
  i][f(!var1 || var2 || !int_arr[0] || int_arr[1] || bool_arr[0] ||
     !bool_arr[1] || get_false() || 0 == sizeof(bool) || false || !true)
    return 5;
  return 0;
}
']
AC_CACHE_CHECK([[whether "bool", "true" and "false" work natively]],[[mhd_cv_bool_native]],
[AC_COMPILE_IFELSE([AC_LANG_SOURCE([[${mhd_bool_test_code}]])],[mhd_cv_bool_native="yes"],
[mhd_cv_bool_native="no"])dnl AC_COMPILE_IFELSE
])
AS_VAR_IF([mhd_cv_bool_native],["yes"],[
AC_DEFINE([[HAVE_BUILTIN_TYPE_BOOL]],[1])],[dnl mhd_cv_bool_native "yes"
AC_CACHE_CHECK([[whether <stdbool.h> is present and defines proper "bool", "true" and "false"]],[[mhd_cv_bool_stdbool]],
[AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
#include <stdbool.h>

${mhd_bool_test_code}
]])],[mhd_cv_bool_stdbool="yes"],[mhd_cv_bool_stdbool="no"])dnl AC_COMPILE_IFELSE
])
dnl end of AC_CACHE_CHECK
AS_VAR_IF([mhd_cv_bool_stdbool],["yes"],[
AC_DEFINE([[HAVE_STDBOOL_H]],[1])
AC_DEFINE([[HAVE_BUILTIN_TYPE_BOOL]],[1])],[dnl mhd_cv_bool_stdbool "yes"
# Need 'bool', 'false' and 'true'.
AC_CHECK_TYPE([bool],[AC_DEFINE([[HAVE_BUILTIN_TYPE_BOOL]], [[1]])],[dnl AC_CHECK_TYPE bool
AC_CHECK_TYPE([_Bool],[AC_DEFINE([[HAVE_BUILTIN_TYPE_BOOL]], [[1]])
AC_DEFINE([[bool]], [[_Bool]])],[AC_DEFINE([[bool]], [[int]])
],[[]])dnl AC_CHECK_TYPE _Bool
],[[]])dnl AC_CHECK_TYPE bool
# Have 'bool'. Need 'false' and 'true'.
AC_CACHE_CHECK([[whether "false" keyword available and works]],[[mhd_cv_keyword_false_works]],
[AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
int main(void)
{
  static bool var1 = false || false;
  static bool var2 = false;
  static bool bool_arr[2 - 3 * ((int) (false))] = {false, !false};
  i][f(var1 || var2 || bool_arr[0] || !bool_arr[1] || false)
    return 5;
  return 0;
}
]])], [[mhd_cv_keyword_false_works='yes']], [[mhd_cv_keyword_false_works='no']])])
dnl end of AC_CACHE_CHECK
AS_VAR_IF([[mhd_cv_keyword_false_works]], [["no"]],[dnl
AC_DEFINE([[false]],[[(0)]], [Define to value interpreted by compiler as boolean "false", i][f "false" is not a keyword and not defined by headers.])])
dnl AS_VAR_IF mhd_cv_keyword_false_works
# Have 'bool' and 'false'. Need 'true'.
AC_CACHE_CHECK([[whether "true" keyword available and works]],[[mhd_cv_keyword_true_works]],
[AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
int main(void)
{
  static bool var1 = true && true;
  static bool var2 = true;
  static bool bool_arr[2 - 3 * ((int) (!true))] = {true, !true};
  i][f(!var1 || !var2 || !bool_arr[0] || bool_arr[1] || !true)
    return 5;
  return 0;
}
]])], [[mhd_cv_keyword_true_works='yes']], [[mhd_cv_keyword_true_works='no']])])
dnl end of AC_CACHE_CHECK
AS_VAR_IF([[mhd_cv_keyword_true_works]], [["no"]],[dnl
AC_DEFINE([[true]],[[(!false)]], [Define to value interpreted by compiler as boolean "true", i][f "true" is not a keyword and not defined by headers.])])
dnl AS_VAR_IF mhd_cv_keyword_false_works
# Have 'bool', 'false' and 'true'.
AC_CACHE_CHECK([[whether the defined "bool", "true" and "false" can work together]],[[mhd_cv_bool_true_false_work]],
[AC_COMPILE_IFELSE([AC_LANG_SOURCE([[${mhd_bool_test_code}]])],[mhd_cv_bool_true_false_work="yes"],
[mhd_cv_bool_true_false_work="no"])dnl AC_COMPILE_IFELSE
])
dnl end of AC_CACHE_CHECK
AS_VAR_IF([[mhd_cv_bool_true_false_work]], [["yes"]],[[:]],[dnl
AC_MSG_FAILURE([cannot find suitable replacements for "bool", "true" and "false"])
])
dnl end of AS_VAR_IF mhd_cv_bool_true_false_work "yes"
])
dnl end of AS_VAR_IF mhd_cv_bool_stdbool "yes"
])
dnl end of AS_VAR_IF mhd_cv_bool_native "yes"
AS_UNSET([mhd_bool_test_code])
m4_newline([[# Expansion of macro $0 ends here]])
])dnl AC_DEFUN MHD_BOOL
