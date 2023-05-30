# SYNOPSIS
#
#   MHD_CHECK_FUNC([FUNCTION_NAME],
#                  [INCLUDES=AC_INCLUDES_DEFAULT], [CHECK_CODE],
#                  [ACTION-IF-AVAILABLE], [ACTION-IF-NOT-AVAILABLE],
#                  [ADDITIONAL_LIBS])
#
# DESCRIPTION
#
#   This macro checks for presence of specific function by including
#   specified headers and compiling and linking CHECK_CODE.
#   This checks both declaration and presence in library.
#   If function is available then macro HAVE_function_name (the name
#   of the function convetedd to all uppercase characters) is defined
#   automatically.
#   Unlike AC_CHECK_FUNCS macro, this macro do not produce false
#   negative result if function is declared with specific calling
#   conventions like __stdcall' or attribute like
#   __attribute__((__dllimport__)) and linker failing to build test
#   program due to the different calling conventions. 
#   By using definition from provided headers, this macro ensures that
#   correct calling convention is used for detection.
#
#   Example usage:
#
#     MHD_CHECK_FUNC([memmem],
#                    [[#include <string.h>]],
#                    [const void *ptr = memmem("aa", 2, "a", 1); (void)ptr;],
#                    [var_use_memmem='yes'], [var_use_memmem='no'])
#
#   The cache variable used in check so if any test will not work
#   correctly on some platform, user may simply fix it by giving cache
#   variable in configure parameters, for example:
#
#     ./configure mhd_cv_func_memmem_have=no
#
#   This simplifies building from source on exotic platforms as patching
#   of configure.ac is not required to change results of tests.
#
# LICENSE
#
#   Copyright (c) 2019-2023 Karlson2k (Evgeny Grin) <k2k@narod.ru>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 5

AC_DEFUN([MHD_CHECK_FUNC],[dnl
AC_PREREQ([2.64])dnl for AS_VAR_IF, m4_ifblank, m4_ifnblank
m4_newline([[# Expansion of $0 macro starts here]])
AC_LANG_ASSERT([C])dnl
m4_ifblank(m4_translit([$1],[()],[  ]), [m4_fatal([First macro argument must not be empty])])dnl
m4_ifblank([$3], [m4_fatal([Third macro argument must not be empty])])dnl
m4_bmatch(m4_normalize([$1]), [\s],dnl
          [m4_fatal([First macro argument must not contain whitespaces])])dnl
m4_if(m4_index([$3], m4_normalize(m4_translit([$1],[()],[  ]))), [-1], dnl
      [m4_fatal([CHECK_CODE parameter (third macro argument) does not contain ']m4_normalize([$1])[' token])])dnl
AS_VAR_PUSHDEF([decl_cv_Var],[ac_cv_have_decl_]m4_bpatsubst(_mhd_norm_expd(m4_translit([$1],[()],[  ])),[[^a-zA-Z0-9]],[_]))dnl
AS_VAR_PUSHDEF([cv_Var],[mhd_cv_func_]m4_bpatsubst(_mhd_norm_expd(m4_translit([$1],[()],[  ])),[[^a-zA-Z0-9]],[_]))dnl
AS_VAR_SET_IF([cv_Var],[],[AC_CHECK_DECL(_mhd_norm_expd([$1]),[],[],[$2])])
AC_CACHE_CHECK([for function $1], [cv_Var],dnl
  [dnl
    AS_VAR_IF([decl_cv_Var],["yes"],dnl
      [dnl
        m4_ifnblank([$6],[dnl
          mhd_check_func_SAVE_LIBS="$LIBS"
          LIBS="_mhd_norm_expd([$6]) $LIBS"
        ])dnl
        AC_LINK_IFELSE(
          [AC_LANG_SOURCE([
m4_default_nblank([$2],[AC_INCLUDES_DEFAULT])

[int main(void)
{

  ]$3[

  return 0;
}
            ]])
          ],
          [AS_VAR_SET([cv_Var],["yes"])],
          [AS_VAR_SET([cv_Var],["no"])]dnl
        )
        m4_ifnblank([$6],[dnl
          LIBS="${mhd_check_func_SAVE_LIBS}"
          AS_UNSET([mhd_check_func_SAVE_LIBS])
        ])dnl
      ],[AS_VAR_SET([cv_Var],["no"])]dnl
    )dnl
  ]dnl
)
AS_VAR_IF([cv_Var], ["yes"],
          [AC_DEFINE([[HAVE_]]m4_bpatsubst(m4_toupper(_mhd_norm_expd(m4_translit([$1],[()],[  ]))),[[^A-Z0-9]],[_]),
                     [1], [Define to 1 if you have the ']_mhd_norm_expd(m4_translit([$1],[()],[  ]))[' function.])
          m4_n([$4])dnl
          ],[$5])
AS_VAR_POPDEF([cv_Var])dnl
AS_VAR_POPDEF([decl_cv_Var])dnl
m4_newline([[# Expansion of $0 macro ends here]])
])dnl AC_DEFUN MHD_CHECK_FUNC
