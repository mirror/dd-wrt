dnl Checks for getrandom support (glibc 2.25+, musl 1.1.20+)
dnl
AC_DEFUN([AC_GETRANDOM], [
    AC_MSG_CHECKING(for getrandom())
    AC_LINK_IFELSE(
		[AC_LANG_PROGRAM([[
		   #include <stdlib.h>  /* for NULL */
		   #include <sys/random.h>
		]],
		[[ return getrandom(NULL, 0U, 0U); ]] )],
		[AC_DEFINE([HAVE_GETRANDOM], [1], [Define to 1 if you have the `getrandom' function.])
		AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])])

	AC_SUBST(HAVE_GETRANDOM)
])
