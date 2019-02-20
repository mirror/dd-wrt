dnl Checks for libxml2.so
AC_DEFUN([AC_LIBXML2], [

  if test "$enable_junction" = yes; then

    dnl look for the library; do not add to LIBS if found
    AC_CHECK_LIB([xml2], [xmlParseFile], [LIBXML2=-lxml2],
                 [AC_MSG_ERROR([libxml2 not found.])])
    AC_SUBST(LIBXML2)

    dnl XXX should also check for presence of xml headers

  fi

])dnl
