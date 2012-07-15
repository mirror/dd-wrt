dnl Copyright (C) 2002, 2004, 2008
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

AC_DEFUN([ENSC_DIET_FIX],
[
	AC_REQUIRE([ENSC_ENABLE_DIETLIBC])

	if test x"${ensc_have_dietlibc}" = xyes; then
		dnl Fix cross-compiler detection of dietlibc since it is
		dnl broken when using names like i386-redhat-linux-gcc
		if test x"$CC" != xgcc; then
			i1=`ls -i $(which $CC) | { read a b; echo $a; }`
			i2=`ls -i $(which gcc) | { read a b; echo $a; }`

			AC_MSG_CHECKING([whether to use gcc instead of $CC])
			if test $i1 = $i2; then
				CC=gcc
				AC_MSG_RESULT(yes)
			else
				AC_MSG_RESULT(no)
			fi
		fi
	fi
])

AC_DEFUN([ENSC_DIET_TYPE_CHECK],
[
	AC_REQUIRE([ENSC_ENABLE_DIETLIBC])
	AH_TEMPLATE($1, [Define if dietlibc defines the '$2' type])

	if test x"${ensc_have_dietlibc}" = xyes; then
		AC_CACHE_CHECK([whether $2 exists in dietlibc],
		       [ensc_cv_type_$1],
		       [AC_LANG_PUSH(C)
			old_cc="${CC}"
			CC="${DIET} ${CC}"
			AC_TRY_COMPILE([$3],
				       [$2	foo; (void)foo;],
				       [ensc_cv_type_$1=yes],
				       [ensc_cv_type_$1=no])
			CC="${old_cc}"
			AC_LANG_POP(C)
		       ])

		if test x"${ensc_cv_type_$1}" = xyes; then
			AC_DEFINE($1, 1)
		fi
	fi
])

AC_DEFUN([ENSC_DIET_CHECK_IN_ADDR_T],
[
	ENSC_DIET_TYPE_CHECK(DIET_HAS_IN_ADDR_T, in_addr_t, [#include <netinet/in.h>])
	ENSC_TYPE_IN_ADDR_T
])

AC_DEFUN([__ENSC_DIET_FUNC_CHECK],
[
	AC_REQUIRE([ENSC_ENABLE_DIETLIBC])
	AH_TEMPLATE(HAVE_DIET_$1, [Define to 1 if dietlibc knows the '$3' function])

	if test x"${ensc_have_dietlibc}" = xyes; then
		AC_CACHE_CHECK([whether dietlibc supports $3 function],
			       [ensc_cv_func_diet_$3],
			       [AC_LANG_PUSH(C)
				old_cc="${CC}"
				CC="${DIET} ${CC}"
				AC_TRY_LINK([$2],
					    [(void)($3($4));],
					    [ensc_cv_func_diet_$3=yes],
					    [ensc_cv_func_diet_$3=no])
				CC="${old_cc}"
				AC_LANG_POP(C)
			       ])

		if test x"$ensc_cv_func_diet_$3" = xyes; then
			AC_DEFINE(HAVE_DIET_$1, 1)
		fi
	fi
])

AC_DEFUN([ENSC_DIET_CHECK_ETHER],
[
	__ENSC_DIET_FUNC_CHECK(ETHER_NTOA,   [#include <netinet/ether.h>], ether_ntoa, 0)
	__ENSC_DIET_FUNC_CHECK(ETHER_ATON,   [#include <netinet/ether.h>], ether_aton, 0)
	__ENSC_DIET_FUNC_CHECK(ETHER_NTOA_R, [#include <netinet/ether.h>], ether_ntoa_r, [0,0])
	__ENSC_DIET_FUNC_CHECK(ETHER_ATON_R, [#include <netinet/ether.h>], ether_aton_r, [0,0])

	cond=true

	for i in ntoa aton ntoa_r aton_r; do
		eval res=\${ether_${i}}
		if test x"$res" = xno; then
			cond=false
			break
		fi
	done

	AM_CONDITIONAL([ENSC_NEED_ETHER], `$cond`)
])

AC_DEFUN([ENSC_DIET_ENV],
[
	AC_REQUIRE([ENSC_ENABLE_DIETLIBC])
	ensc_diet_env_old_cc=$CC
	CC="${DIET} ${CC}"
	$1
	CC=$ensc_diet_env_old_cc
])
