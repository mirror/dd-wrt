dnl $Id: ensc_changelog.m4,v 1.1 2003/12/12 00:47:16 ensc Exp $

dnl Copyright (C) 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
dnl  
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; version 2 of the License.
dnl  
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl  
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

dnl Usage: ENSC_CHANGELOG(<CVS_TAG>)

AC_DEFUN([ENSC_CHANGELOG],
[
	if test x"$1" != x; then
		AC_SUBST(CVS2CL_TAG, ['-F $1'])
	fi

	AC_CHECK_PROGS(CVS2CL, [cvs2cl])
	AM_CONDITIONAL(HAVE_CVS2CL,  [test x"$CVS2CL" != x])

	AC_CHECK_PROGS(RCS2LOG, [rcs2log])
	AM_CONDITIONAL(HAVE_RCS2LOG,  [test x"$RCS2LOG" != x])
])
