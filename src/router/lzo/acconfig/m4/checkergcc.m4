## --------------------------------------------------------- ##
## Check for checkergcc.                                     ##
## Adapted from AC_PROG_CC_GNU.                              ##
## --------------------------------------------------------- ##

# serial 1

AC_DEFUN(mfx_PROG_CC_CHECKERGCC,
[AC_CACHE_CHECK([whether we are using checkergcc], mfx_cv_prog_checkergcc,
[dnl The semicolon is to pacify NeXT's syntax-checking cpp.
cat > conftest.c <<EOF
#ifdef __GNUC__
#ifdef __CHECKER__
  yes;
#endif
#endif
EOF
if AC_TRY_COMMAND(${CC-cc} -E conftest.c) | egrep yes >/dev/null 2>&1; then
  mfx_cv_prog_checkergcc=yes
else
  mfx_cv_prog_checkergcc=no
fi])])
