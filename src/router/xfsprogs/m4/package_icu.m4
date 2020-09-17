AC_DEFUN([AC_HAVE_LIBICU],
  [ PKG_CHECK_MODULES([libicu], [icu-i18n], [have_libicu=yes], [have_libicu=no])
    AC_SUBST(have_libicu)
    AC_SUBST(libicu_CFLAGS)
    AC_SUBST(libicu_LIBS)
  ])
