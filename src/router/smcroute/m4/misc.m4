# Misc. helper macros

AC_DEFUN([AC_CHECK_SUN_LEN],[
	AC_MSG_CHECKING(for sun_len member in struct sockaddr_un)
	AC_COMPILE_IFELSE([
		AC_LANG_PROGRAM([[
			#include <sys/un.h>
		]],[[
			struct sockaddr_un dummy;
			dummy.sun_len = 0;
		]])],[
		AC_DEFINE(HAVE_SOCKADDR_UN_SUN_LEN, 1, [Define if the struct sockaddr_un has a member sun_len on your OS])
		AC_MSG_RESULT(yes)],[
		AC_MSG_RESULT(no)])
	])

AC_DEFUN([AC_CHECK_SIN_LEN],[
	AC_MSG_CHECKING(for sin_len member in struct sockaddr_in)
	AC_COMPILE_IFELSE([
		AC_LANG_PROGRAM([[
			#include <netinet/in.h>
		]],[[
			struct sockaddr_in dummy;
			dummy.sin_len = 0;
		]])],[
		AC_DEFINE(HAVE_SOCKADDR_IN_SIN_LEN, 1, [Define if the struct sockaddr_in has a member sin_len on your OS])
		AC_MSG_RESULT(yes)],[
		AC_MSG_RESULT(no)])
	])
