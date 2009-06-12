dnl See whether weak symbol is supported.
AC_DEFUN(LIBIBERTY_HAVE_WEAK_SYMBOL,
[AC_MSG_CHECKING([whether weak symbol works])
AC_CACHE_VAL(libiberty_cv_have_weak_symbol,
[AC_TRY_COMPILE([
#pragma weak liberty_weak_symbol_test
extern void liberty_weak_symbol_test ();],
[if (return &liberty_weak_symbol_test != NULL) return 1],
libiberty_cv_have_weak_symbol=no, libiberty_cv_have_weak_symbol=yes)])
AC_MSG_RESULT($libiberty_cv_have_weak_symbol)
if test $libiberty_cv_have_weak_symbol = yes; then
  AC_DEFINE(HAVE_WEAK_SYMBOL, 1, [Define if weak symbol works.])
fi
])dnl
