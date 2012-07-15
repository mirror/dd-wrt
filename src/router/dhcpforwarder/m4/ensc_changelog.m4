dnl Copyright (C) 2002, 2003, 2008
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

dnl Usage: ENSC_CHANGELOG(<VC_TAG>, [<style>])

AC_DEFUN([ENSC_CHANGELOG],
[
	if test x"$1" != x; then
		AC_SUBST(CVS2CL_TAG, ['-F $1'])
		AC_SUBST(SVN_TAG,    [$1])
		AC_SUBST(SCM_TAG,    [$1])
	fi

	AC_CHECK_PROGS(CVS2CL, [cvs2cl])
	AM_CONDITIONAL(HAVE_CVS2CL,  [test x"$CVS2CL" != x])

	AC_CHECK_PROGS(RCS2LOG, [rcs2log])
	AM_CONDITIONAL(HAVE_RCS2LOG,  [test x"$RCS2LOG" != x])

	AC_CHECK_PROGS(SVN, [svn])
	AM_CONDITIONAL(HAVE_SVN,      [test x"$SVN" != x])

	AC_CHECK_PROGS(GIT, [git])
	AM_CONDITIONAL(HAVE_GIT,      [test x"$GIT" != x])

	AM_CONDITIONAL(IS_CVS_WORKDIR, [test -d CVS  -a \( cvs = "$2" -o x = x"$2" \)])
	AM_CONDITIONAL(IS_SVN_WORKDIR, [test -d .svn -a \( svn = "$2" -o x = x"$2" \)])
	AM_CONDITIONAL(IS_GIT_WORKDIR, [test -d .git -a \( git = "$2" -o x = x"$2" \)])
])
