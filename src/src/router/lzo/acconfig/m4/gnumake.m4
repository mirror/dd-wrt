## --------------------------------------------------------- ##
## Check for GNU make (do not cache this setting)            ##
## Adapted from AM_MISSING_PROG and AM_PROG_LD_GNU.          ##
## --------------------------------------------------------- ##

# serial 1

dnl mfx_PROG_MAKE_GNU(NAME)
AC_DEFUN(mfx_PROG_MAKE_GNU,
[AC_MSG_CHECKING([whether we are using GNU make])
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
if (${MAKE-make} --version) 2>&1 </dev/null | grep 'GNU' >/dev/null; then
   $1=yes
   AC_MSG_RESULT(yes)
else
   $1=no
   AC_MSG_RESULT(no)
fi
AC_SUBST($1)])
