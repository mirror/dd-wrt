dnl Checks for pthreads library and headers
dnl
AC_DEFUN([AC_LIBPTHREAD], [

    dnl Check for library, but do not add -lpthreads to LIBS
    AC_CHECK_LIB([pthread], [pthread_create],
		[AC_DEFINE([HAVE_LIBPTHREAD], [1],
				[Define to 1 if you have libpthread.])
		AC_CHECK_HEADERS([pthread.h], [],
				[AC_MSG_ERROR([libpthread headers not found.])])
		 AC_SUBST([LIBPTHREAD],[-lpthread])],
                 [$1])

])dnl
