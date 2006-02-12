## --------------------------------------------------------- ##
## Check if gcc accepts '-pipe'.                             ##
## Adapted from AC_PROG_CC and AC_PROG_GCC_TRADITIONAL.      ##
## --------------------------------------------------------- ##

# serial 1

AC_DEFUN(mfx_PROG_GCC_PIPE,
[AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CPP])dnl
if test "$ac_cv_prog_gcc" = yes; then
AC_CACHE_CHECK([whether ${CC-cc} accepts -pipe], mfx_cv_prog_gcc_pipe,
[echo 'int main() { return 0; }' > conftest.c
if test -z "`${CC-cc} -pipe -c conftest.c 2>&1`"; then
  mfx_cv_prog_gcc_pipe=yes
else
  mfx_cv_prog_gcc_pipe=no
fi
rm -f conftest*
])
  if test "$mfx_cv_prog_gcc_pipe" = yes; then
    CC="$CC -pipe"
  fi
fi
])
