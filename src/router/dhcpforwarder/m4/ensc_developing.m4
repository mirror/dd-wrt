dnl Copyright (C) 2003, 2004, 2008
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

AC_DEFUN([ENSC_DEVELOPING],
[
	AC_PATH_PROGS(TR, tr)
	AC_PATH_PROGS(ECHO, echo)

	default=$2
	: ${default:=no}

	AC_MSG_CHECKING([whether to enable developing])
	AC_ARG_ENABLE(developing,
		      [AC_HELP_STRING([--enable-developing],
				      [Use settings for developing; DO
				      NOT use in final build (default:
				      OFF)])],
		      [], [ enable_developing=${default} ])

	enable_developing=`${ECHO} ${enable_developing} | ${TR} A-Z a-z`

	if test x"${enable_developing}" = xno; then
		$1=no
		AC_MSG_RESULT(no)
		DEVELOPING_CFLAGS=
	else
		$1=yes
		AC_MSG_RESULT(yes)
		enable_stl_malloc_default=yes

		DEVELOPING_STRICT_CFLAGS=['-Wold-style-cast -Wunreachable-code -Wshadow']
		DEVELOPING_CFLAGS=['-W -Werror -g3 -O0 -Woverloaded-virtual -Wsign-promo -Wsynth
				    -Wchar-subscripts -Wparentheses -Wsequence-point -Wswitch
				    -Wfloat-equal -Wpointer-arith']
	fi

	AC_SUBST([$1])
])

AC_DEFUN([__ENSC_RELEASE],
[
	default=${ensc_is_release_default:-no}
	: ${default:=no}

	AC_MSG_CHECKING(whether to enable release-mode)
	AC_ARG_ENABLE(release,
		      AC_HELP_STRING([--disable-release],
				     [disable settings which are creating faster/smaller code]),
		      [case "${enableval}" in
			  yes|no) enable_release="${enableval}";;
			  *)      AC_MSG_ERROR([bad value ${enableval} for --disable-release]) ;;
		       esac],
		      [ enable_release=${default} ])

	if test x"${enable_release}" = xno; then
		ensc_is_release=no
		RELEASE_CFLAGS=
	else
		ensc_is_release=yes
		RELEASE_CFLAGS=
		AC_DEFINE(NDEBUG, [(1==1)], [The NDEBUG macro as specified in C99, 7.2])
	fi
])

AC_DEFUN([ENSC_RELEASE],
[
	ensc_is_release_default=$2
	AC_REQUIRE([__ENSC_RELEASE])

	$1=${ensc_is_release}
	AC_SUBST([$1])
	AC_MSG_RESULT([$$1])
])
