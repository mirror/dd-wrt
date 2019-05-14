dnl Checks for libxml2.so
AC_DEFUN([AC_LIBXML2], [

  PKG_PROG_PKG_CONFIG([0.9.0])
  AS_IF(
    [test "$enable_junction" = "yes"],
    [PKG_CHECK_MODULES([XML2], [libxml-2.0 >= 2.4],
                       [LIBXML2="${XML2_LIBS}"
                        AM_CPPFLAGS="${AM_CPPFLAGS} ${XML2_CFLAGS}"
                        AC_DEFINE([HAVE_LIBXML2], [1],
                                  [Define to 1 if you have and wish to use libxml2.])],
                       [AC_MSG_ERROR([libxml2 not found.])])])

  AC_SUBST([AM_CPPFLAGS])
  AC_SUBST(LIBXML2)

])dnl
