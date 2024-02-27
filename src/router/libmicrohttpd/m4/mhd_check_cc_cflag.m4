# SYNOPSIS
#
#   MHD_CHECK_CC_CFLAG([FLAG-TO-TEST], [VARIABLE-TO-PREPEND-CFLAGS],
#                      [ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED])
#
# DESCRIPTION
#
#   This macro checks whether the specific compiler flag is supported.
#   The check is performing by prepending FLAG-TO-TEST to CFLAGS, then
#   prepending value of VARIABLE-TO-PREPEND-CFLAGS (if any) to CFLAGS, and
#   then performing compile and link test. If test succeed without warnings,
#   then the flag is considered to be supported. Otherwise, if compile and link
#   without test flag can be done without any warning, the flag is considered
#   to be unsuppoted.
#
#   Example usage:
#
#     MHD_CHECK_CC_CFLAG([-Wshadow], [additional_CFLAGS],
#                        [additional_CFLAGS="${additional_CFLAGS} -Wshadow"])
#
#   Defined cache variable used in check so if any test will not work
#   correctly on some platform, user may simply fix it by giving cache
#   variable in configure parameters, for example:
#
#     ./configure mhd_cv_cc_fl_supp__Wshadow=no
#
#   This simplify building from source on exotic platforms as patching
#   of configure.ac is not required to change results of tests.
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

AC_DEFUN([MHD_CHECK_CC_CFLAG],[dnl
_MHD_CHECK_CC_XFLAG([$1],[$2],[$3],[$4],[[CFLAGS]])dnl
])

# SYNOPSIS
#
#   _MHD_CHECK_CC_XFLAG([FLAG-TO-TEST], [VARIABLE-TO-PREPEND-CFLAGS],
#                       [ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED],
#                       [CFLAGS|LDFLAGS])
#
AC_DEFUN([_MHD_CHECK_CC_XFLAG],[dnl
  AC_PREREQ([2.64])dnl for AS_VAR_IF, m4_ifnblank
  AC_LANG_ASSERT([C])dnl
  AC_REQUIRE([AC_PROG_CC])dnl
  m4_ifblank([$1], [m4_fatal([$0: First macro argument must not be empty])])dnl
  m4_bmatch(m4_normalize([$1]), [\s],dnl
            [m4_fatal([$0: First macro argument must not contain whitespaces])])dnl
  m4_bmatch(_mhd_norm_expd([$2]), [\s],dnl
            [m4_fatal([$0: Second macro argument must not contain whitespaces])])dnl
  m4_bmatch([$1], [\$], [m4_fatal([$0: First macro argument must not contain '$'])])dnl
  m4_bmatch([$2], [\$], [m4_fatal([$0: Second macro argument must not contain '$'])])dnl
  AC_REQUIRE([MHD_FIND_CC_XFLAG_WARNPARAMS])dnl sets 'mhd_xFLAGS_params_warn' variable
  m4_bmatch(_mhd_norm_expd([$5]), [^\(CFLAGS\|LDFLAGS\)$],[],dnl
   [m4_fatal([$0: Fifth macro argument must be either 'CFLAGS' or 'LDFLAGS; ']_mhd_norm_expd([$5])[' is not supported])])dnl
  _MHD_CHECK_CC_XFLAG_BODY([$1],[$2],[$3],[$4],[mhd_xFLAGS_params_warn],[$5])dnl
])


# SYNOPSIS
#
#   _MHD_CHECK_CC_XFLAG_BODY([FLAG-TO-TEST], [VARIABLE-TO-PREPEND-CFLAGS],
#                            [ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED],
#                            [VARIABLE-TO-ENABLE-WARNS], [CFLAGS|LDFLAGS])
#
AC_DEFUN([_MHD_CHECK_CC_XFLAG_BODY],[dnl
  AC_LANG_ASSERT([C])dnl
  m4_ifblank([$1], [m4_fatal([$0: First macro argument must not be empty])])dnl
  m4_bmatch(_mhd_norm_expd([$1]), [\s],dnl
            [m4_fatal([$0: First macro argument must not contain whitespaces])])dnl
  m4_bmatch(_mhd_norm_expd([$2]), [\s],dnl
            [m4_fatal([$0: Second macro argument must not contain whitespaces])])dnl
  m4_bmatch([$1], [\$], [m4_fatal([$0: First macro argument must not contain '$'])])dnl
  m4_bmatch([$2], [\$], [m4_fatal([$0: Second macro argument must not contain '$'])])dnl
  m4_ifblank([$5], [m4_fatal([$0: Fifth macro argument must not be empty])])dnl
  m4_bmatch([$5], [\$], [m4_fatal([$0: Fifth macro argument must not contain '$'])])dnl
  m4_bmatch(_mhd_norm_expd([$6]), [^\(CFLAGS\|LDFLAGS\)$],[],dnl
   [m4_fatal([$0: Sixth macro argument must be either 'CFLAGS' or 'LDFLAGS; ']_mhd_norm_expd([$6])[' is not supported])])dnl
  m4_pushdef([XFLAGS],_mhd_norm_expd([$6]))dnl
  dnl Keep uppercase letters to avoid clashes for parameters like -fPIE and -fpie
  AS_VAR_PUSHDEF([cv_Var],[mhd_cv_cc_]m4_tolower(m4_substr(_mhd_norm_expd([$6]),0,1))[fl_supp_]m4_bpatsubst(_mhd_norm_expd([$1]),[[^a-zA-Z0-9]],[_]))dnl

  AC_CACHE_CHECK([whether $[]CC supports _mhd_norm_expd([$1]) flag], cv_Var,
    [dnl
      AS_VAR_PUSHDEF([save_xFLAGS_Var], [mhd_check_cc_flag_save_]XFLAGS)dnl
      AS_VAR_SET([save_xFLAGS_Var],["${XFLAGS}"])
      m4_ifnblank([$2],[dnl
        m4_if(_mhd_norm_expd([$2]),[XFLAGS],
          [XFLAGS="${save_xFLAGS_Var} _mhd_norm_expd([$1]) $[]{_mhd_norm_expd([$5])}"],
          [XFLAGS="$[]{_mhd_norm_expd([$2])} _mhd_norm_expd([$1]) ${save_xFLAGS_Var} $[]{_mhd_norm_expd([$5])}"])
      ],[dnl
        XFLAGS="_mhd_norm_expd([$1]) $[]XFLAGS $[]{_mhd_norm_expd([$5])}"
      ])dnl
      mhd_check_cc_flag_save_c_werror_flag="$ac_c_werror_flag"
      ac_c_werror_flag=yes
      [#] Reuse the same source for all the checks here
      AC_LANG_CONFTEST([AC_LANG_SOURCE([[
int main(void)
{
  return 0;
}
      ]])])
      AC_LINK_IFELSE([],
        [AS_VAR_SET([cv_Var],["yes"])],
        [ [#] Compile and link failed with test flag added
          m4_ifnblank([$2],[dnl
            m4_if(_mhd_norm_expd([$2]),[XFLAGS],
              [XFLAGS="${save_xFLAGS_Var} $[]{_mhd_norm_expd([$5])}"],
              [XFLAGS="$[]{_mhd_norm_expd([$2])} ${save_xFLAGS_Var} $[]{_mhd_norm_expd([$5])}"])
          ],[dnl
            XFLAGS="${save_xFLAGS_Var} $[]{_mhd_norm_expd([$5])}"
            ])dnl
          AC_LINK_IFELSE([],
            [AS_VAR_SET([cv_Var],["no"])],
            [ [#] Compile and link failed with test flag removed as well
              m4_ifnblank([$2],[dnl
                m4_if(_mhd_norm_expd([$2]),[XFLAGS],
                  [XFLAGS="${save_xFLAGS_Var} _mhd_norm_expd([$1])"],
                  [XFLAGS="$[]{_mhd_norm_expd([$2])} _mhd_norm_expd([$1]) ${save_xFLAGS_Var}"])
              ],[dnl
                XFLAGS="_mhd_norm_expd([$1]) ${save_xFLAGS_Var}"
              ])dnl
              ac_c_werror_flag="$mhd_check_cc_flag_save_c_werror_flag"
              AC_LINK_IFELSE([],
                [AS_VAR_SET([cv_Var],["yes"])],
                [AS_VAR_SET([cv_Var],["no"])],
              )
            ]
          )
        ]
      )
      ac_c_werror_flag="$mhd_check_cc_flag_save_c_werror_flag"
      AS_VAR_SET([XFLAGS],["${save_xFLAGS_Var}"])
      AS_UNSET(save_xFLAGS_Var)
      AS_VAR_POPDEF([save_xFLAGS_Var])dnl
    ]
  )
  m4_ifnblank([$3$4],[dnl
    AS_VAR_IF([cv_Var], ["yes"], [$3], m4_default_nblank([$4]))
    ])dnl
  AS_VAR_POPDEF([cv_Var])dnl
  m4_popdef([XFLAGS])dnl
])


#
# SYNOPSIS
#
#   MHD_FIND_CC_XFLAG_WARNPARAMS()
#
AC_DEFUN([MHD_FIND_CC_XFLAG_WARNPARAMS],[dnl
  AC_LANG_ASSERT([C])dnl
  AC_REQUIRE([MHD_FIND_CC_CFLAG_WWARN])dnl
  mhd_xFLAGS_params_warn=''
  _MHD_CHECK_CC_XFLAG_BODY([-Wunused-command-line-argument],[],
    [
      MHD_APPEND_FLAG_TO_VAR([mhd_xFLAGS_params_warn],[-Wunused-command-line-argument])
    ],[],[mhd_cv_cc_flag_Wwarn],[[CFLAGS]]
  )
  _MHD_CHECK_CC_XFLAG_BODY([-Wignored-optimization-argument],[],
    [
      MHD_APPEND_FLAG_TO_VAR([mhd_xFLAGS_params_warn],[-Wignored-optimization-argument])
    ],[],[mhd_cv_cc_flag_Wwarn],[[CFLAGS]]
  )
  _MHD_CHECK_CC_XFLAG_BODY([-Winvalid-command-line-argument],[],
    [
      MHD_APPEND_FLAG_TO_VAR([mhd_xFLAGS_params_warn],[-Winvalid-command-line-argument])
    ],[],[mhd_cv_cc_flag_Wwarn],[[CFLAGS]]
  )
  _MHD_CHECK_CC_XFLAG_BODY([-Wunknown-argument],[],
    [
      MHD_APPEND_FLAG_TO_VAR([mhd_xFLAGS_params_warn],[-Wunknown-argument])
    ],[],[mhd_cv_cc_flag_Wwarn],[[CFLAGS]]
  )
  MHD_PREPEND_FLAG_TO_VAR([mhd_xFLAGS_params_warn],[$mhd_cv_cc_flag_Wwarn])
])


#
# SYNOPSIS
#
#   MHD_FIND_CC_CFLAG_WWARN()
#
AC_DEFUN([MHD_FIND_CC_CFLAG_WWARN],[dnl
  AC_PREREQ([2.64])dnl for AS_VAR_IF, m4_ifnblank
  AC_REQUIRE([AC_PROG_CC])dnl
  AC_LANG_ASSERT([C])dnl
  AC_MSG_CHECKING([for $[]CC flag to warn on unknown -W parameters])
  AC_CACHE_VAL([mhd_cv_cc_flag_Wwarn],
    [
      mhd_check_cc_flagwarn_save_c_werror_flag="$ac_c_werror_flag"
      ac_c_werror_flag=yes
      mhd_find_cc_Wwarn_save_CFLAGS="$CFLAGS"
      AS_UNSET([mhd_cv_cc_flag_Wwarn])
      for mhd_cv_cc_flag_Wwarn in '' '-Wunknown-warning-option' '-Werror=unknown-warning-option' ; do
        AS_VAR_IF([mhd_cv_cc_flag_Wwarn], ["unknown"], [break])
        [#] Reuse the same source for all the checks here
        AC_LANG_CONFTEST([AC_LANG_SOURCE([[
int main(void)
{
  return 0;
}
        ]])])
        CFLAGS="-Wmhd-noexist-flag $mhd_find_cc_Wwarn_save_CFLAGS $mhd_cv_cc_flag_Wwarn"
        AC_LINK_IFELSE([],
          [],
          [ [#] Compile and link failed if test flag and non-existing flag added
            CFLAGS="$mhd_find_cc_Wwarn_save_CFLAGS $mhd_cv_cc_flag_Wwarn"
            AC_LINK_IFELSE([],
              [ [#] Compile and link succeed if only test flag added
                break
              ]
            )
          ]
        )
      done
      CFLAGS="$mhd_find_cc_Wwarn_save_CFLAGS"
      AS_VAR_IF([mhd_cv_cc_flag_Wwarn], ["unknown"],
        [
          _AS_ECHO_LOG([No suitable flags detected. Check whether default flags are correct.])
          AC_LINK_IFELSE([AC_LANG_SOURCE([[
int main(void)
{
  return 0;
}
            ]])],
            [:],
            [ [#] Compile and link fails (or warns) with default flags
              AC_MSG_WARN([Compiler warns (of fails) with default flags!])
              AC_MSG_WARN([Check whether compiler and compiler flags are correct.])
            ]
          )
        ]
      )
      CFLAGS="$mhd_find_cc_Wwarn_save_CFLAGS"
      AS_UNSET([mhd_find_cc_Wwarn_save_CFLAGS])
      ac_c_werror_flag="$mhd_check_cc_flagwarn_save_c_werror_flag"
      AS_UNSET([mhd_check_cc_flagwarn_save_c_werror_flag])
    ]
  )
  AS_IF([test -z "$mhd_cv_cc_flag_Wwarn"],
    [AC_MSG_RESULT([none needed])],
    [AC_MSG_RESULT([$mhd_cv_cc_flag_Wwarn])])
  AS_VAR_IF([mhd_cv_cc_flag_Wwarn], ["unknown"],
    [AC_MSG_WARN([Unable to find compiler flags to warn on unsupported -W options. Final compiler options may be suboptimal.])])

])
