## --------------------------------------------------------- ##
## Check if gcc suffers the '-fstrength-reduce' bug.         ##
## --------------------------------------------------------- ##

# serial 1

AC_DEFUN(mfx_PROG_GCC_BUG_STRENGTH_REDUCE,
[AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CPP])dnl
if test "$ac_cv_prog_gcc" = yes; then
mfx_save_cflags="$CFLAGS"
CFLAGS="-O2 -fstrength-reduce"
AC_CACHE_CHECK([whether ${CC-cc} suffers the -fstrength-reduce bug],
mfx_cv_prog_gcc_bug_strength_reduce,
[AC_TRY_RUN([
/* gcc strength-reduce optimization bug on Intel platforms.
 * Adapted from bug-report by John E. Davis <davis@space.mit.edu>
 * Compile and run it using gcc -O2 -fno-strength-reduce and
 * gcc -O2 -fstrength-reduce.
 */
int a[3]; unsigned an = 3;
int strength_reduce_bug();
int main() { unsigned j;
  for (j = 0; j < an; j++)
    a[j] = (int)j - 3;
  return strength_reduce_bug(); }
int strength_reduce_bug()
{ return a[0] != -3 || a[1] != -2 || a[2] != -1; }],
[mfx_cv_prog_gcc_bug_strength_reduce=no],
[mfx_cv_prog_gcc_bug_strength_reduce=yes],
[mfx_cv_prog_gcc_bug_strength_reduce=unknown])])
CFLAGS="$mfx_save_cflags"
fi
])
