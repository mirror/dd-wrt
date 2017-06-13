#
# Autoconf input file.
#
# Version:	$Id: 6c0309e05fd8bd0d45849952abf35b22a7f394bd $
#

AC_PREREQ([2.59])
AC_INIT(rlm_sqlippool.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_sqlippool])

if test x$with_[]modname != xno; then

	dnl  This module doesn't need any autoconf test which is not already
	dnl  in top-level configure.

	targetname=modname
else
	targetname=
	echo \*\*\* module modname is disabled.
fi

mod_ldflags="$SMART_LIBS"
mod_cflags="$SMART_CPPFLAGS"
AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)
