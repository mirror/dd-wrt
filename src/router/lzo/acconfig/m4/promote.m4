## --------------------------------------------------------- ##
## Check how the compiler promotes integrals.                ##
## --------------------------------------------------------- ##

# serial 1

AC_DEFUN(mfx_PROG_CC_INTEGRAL_PROMOTION,
[AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CPP])dnl
AC_CACHE_CHECK([how the compiler promotes integrals],
mfx_cv_prog_cc_integral_promotion,
[AC_TRY_COMPILE([], [
char x[1 - 2 * ( (((unsigned char)128) << (int)(8*sizeof(int)-8)) >= 0 )];
],
mfx_cv_prog_cc_integral_promotion="ANSI C (value-preserving)",
mfx_cv_prog_cc_integral_promotion="Classic (unsigned-preserving)")])
])
