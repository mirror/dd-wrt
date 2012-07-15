dnl Copyright (C) 2002, 2008
dnl               Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; version 2 and/or 3 of the License.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program. If not, see http://www.gnu.org/licenses/.

dnl Usage: ENSC_INITRDDIR(<variable>)
AC_DEFUN([ENSC_INITRDDIR],
[
	AC_MSG_CHECKING([for initrddir])
	AC_ARG_WITH([initrddir],
		    [AC_HELP_STRING([--with-initrddir <DIR>],
				    [use <DIR> as directory for SysV init-files (default: $sysconfdir/init.d)])],
		    [case "$withval" in
			yes|no)	AC_MSG_ERROR(['$withval' is not a valid value for '--with-initrddir']);;
			*)      ensc_initrddir=$withval;;
		     esac],
		    [ensc_initrddir='$(sysconfdir)/init.d'])

	if test "$1"; then
		$1=$ensc_initrddir
		AC_SUBST($1)
	fi

	AC_MSG_RESULT($ensc_initrddir)
])
