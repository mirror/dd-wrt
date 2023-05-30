# SYNOPSIS
#
#   MHD_FIND_LIB([FUNCTION_NAME],
#                [INCLUDES=AC_INCLUDES_DEFAULT], [CHECK_CODE],
#                [LIBS_TO_CHECK],
#                [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                [VAR_TO_PREPEND_LIB=LIBS], [ADDITIONAL_LIBS])
#
# DESCRIPTION
#
#   This macro checks for presence of specific function by including
#   specified headers and compiling and linking CHECK_CODE.
#   This checks both the declaration and the presence in library.
#   If declaration is not found in headers then libraries are not
#   checked.
#   LIBS_TO_CHECK is whitespace-separated list of libraries to check.
#   The macro first tries to link without any library, and if it fails
#   the libraries are checked one by one. 
#   The required library (if any) prepended to VAR_TO_PREPEND_LIB (or
#   to the LIBS variable if VAR_TO_APPEND_LIB is not specified).
#   By using definition from provided headers, this macro ensures that
#   correct calling convention is used for detection.
#
#   Example usage:
#
#     MHD_FIND_LIB([clock_gettime],
#                  [[#include <time.h>]],
#                  [[struct timespec tp;
#                    if (0 > clock_gettime(CLOCK_REALTIME, &tp)) return 3;]],
#                  [rt],
#                  [var_use_gettime='yes'],[var_use_gettime='no'])
#
#   Defined cache variable used in the check so if any test will not
#   work correctly on some platform, a user may simply fix it by giving
#   cache variable in configure parameters, for example:
#
#     ./configure mhd_cv_find_clock_gettime=no
#
#   This simplifies building from source on exotic platforms as patching
#   of configure.ac is not required to change results of tests.
#
# LICENSE
#
#   Copyright (c) 2023 Karlson2k (Evgeny Grin) <k2k@narod.ru>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([MHD_FIND_LIB],[dnl
AC_PREREQ([2.64])dnl for AS_VAR_IF, m4_ifblank, m4_ifnblank
m4_newline([[# Expansion of $0 macro starts here]])
AC_LANG_ASSERT([C])dnl
m4_ifblank(m4_translit([$1],[()],[  ]), [m4_fatal([First macro argument FUNCTION_NAME must not be empty])])dnl
m4_ifblank([$3], [m4_fatal([Third macro argument CHECK_CODE must not be empty])])dnl
m4_bmatch(m4_normalize([$1]), [\s],dnl
          [m4_fatal([First macro argument FUNCTION_NAME must not contain whitespaces])])dnl
m4_if(m4_index([$3], m4_normalize(m4_translit([$1],[()],[  ]))), [-1], dnl
      [m4_fatal([CHECK_CODE parameter (third macro argument) does not contain ']m4_normalize([$1])[' token])])dnl
m4_bmatch([$7], [\$], [m4_fatal([$0: Seventh macro argument VAR_TO_PREPEND_LIB must not contain '$'])])dnl
_MHD_FIND_LIB_BODY([$1],[$2],[$3],[$4],[$5],[$6],m4_default_nblank(_mhd_norm_expd([$7]),[LIBS]),[$8])dnl
])dnl AC_DEFUN MHD_FIND_LIB

# SYNOPSIS
#
#   _MHD_FIND_LIB_BODY([1_FUNCTION_NAME],
#                      [2_INCLUDES=AC_INCLUDES_DEFAULT], [3_CHECK_CODE],
#                      [4_LIBS_TO_CHECK],
#                      [5_ACTION-IF-FOUND], [6_ACTION-IF-NOT-FOUND],
#                      [7_VAR_TO_PREPEND_LIB=LIBS], [8_ADDITIONAL_LIBS])

AC_DEFUN([_MHD_FIND_LIB_BODY],[dnl
AS_VAR_PUSHDEF([decl_cv_Var],[ac_cv_have_decl_]m4_bpatsubst(_mhd_norm_expd(m4_translit([$1],[()],[  ])),[[^a-zA-Z0-9]],[_]))dnl
AS_VAR_PUSHDEF([cv_Var], [mhd_cv_find_lib_]m4_bpatsubst(_mhd_norm_expd(m4_translit([$1],[()],[  ])),[[^a-zA-Z0-9]],[_]))dnl
AS_VAR_SET_IF([cv_Var],[],[AC_CHECK_DECL(_mhd_norm_expd([$1]),[],[],[$2])])
AC_CACHE_CHECK([for library containing function $1], [cv_Var],
  [
    AS_VAR_IF([decl_cv_Var],["yes"],dnl
      [dnl
        mhd_find_lib_SAVE_LIBS="$LIBS"
        m4_if([$7],LIBS,[dnl
            mhd_find_lib_CHECK_LIBS="_mhd_norm_expd([$8]) $LIBS"
          ],[dnl
            mhd_find_lib_CHECK_LIBS="[$]$7 _mhd_norm_expd([$8]) $LIBS"
          ]
        )
        # Reuse the same source file
        AC_LANG_CONFTEST(
          [AC_LANG_SOURCE([
m4_default_nblank([$2],[AC_INCLUDES_DEFAULT])

[int main(void)
{

  ]$3[

  return 0;
}
             ]])
          ]
        )
        for mhd_find_lib_LIB in '' $4
        do
          AS_IF([test -z "${mhd_find_lib_LIB}"],
            [LIBS="${mhd_find_lib_CHECK_LIBS}"],
            [LIBS="-l${mhd_find_lib_LIB} ${mhd_find_lib_CHECK_LIBS}"]
          )
          AC_LINK_IFELSE([],
            [
              AS_IF([test -z "${mhd_find_lib_LIB}"],
                [AS_VAR_SET([cv_Var],["none required"])],
                [AS_VAR_SET([cv_Var],["-l${mhd_find_lib_LIB}"])]
              )
            ]
          )
          AS_VAR_SET_IF([cv_Var],[break])
        done
        AS_UNSET([mhd_find_lib_LIB])
        rm -f conftest.$ac_ext
        LIBS="${mhd_find_lib_SAVE_LIBS}"
        AS_UNSET([mhd_find_lib_SAVE_LIBS])
      ]
    )
    AS_VAR_SET_IF([cv_Var],[:],[AS_VAR_SET([cv_Var],["no"])])
  ]
)
AS_IF([test "x${cv_Var}" != "xno"],
[dnl
  AS_VAR_IF([cv_Var],["none required"],[:],
    [
      AS_IF([test -z "[$]$7"],[$7="${cv_Var}"],[$7="${cv_Var} [$]$7"])
    ]
  )
  m4_n([$5])dnl
],[$6])dnl AS_VAR_SET_IF cv_Var
AS_VAR_POPDEF([cv_Var])dnl
AS_VAR_POPDEF([decl_cv_Var])dnl
m4_newline([[# Expansion of $0 macro ends here]])
])dnl AC_DEFUN MHD_CHECK_FUNC
