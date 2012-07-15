dnl Copyright (C) 2002, 2004, 2008, 2012
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

AC_DEFUN([ENSC_TYPE_IN_ADDR_T],
[
	AC_CACHE_CHECK([whether type in_addr_t is known],
		       [ensc_cv_type_in_addr_t],
		       [AC_LANG_PUSH(C)
		AC_TRY_COMPILE([#include <netinet/in.h>
		       ],
				       [volatile in_addr_t	foo;
					foo = 0],
				       [ensc_cv_type_in_addr_t=yes],
			       [ensc_cv_type_in_addr_t=no])
			AC_LANG_POP(C)
		       ])


	if test x"${ensc_cv_type_in_addr_t}" = xyes; then
		AC_DEFINE([HAVE_IN_ADDR_T], 1,
			  [Define if in_addr_t type exists])
	fi
])

AC_DEFUN([ENSC_BROKEN_RLIMIT_PROTOS],
[
	AC_CACHE_CHECK([whether *etrlimit() are having broken prototypes],
		       [ensc_cv_sys_broken_rlimit_protos],
		       [AC_LANG_PUSH(C)
			AC_TRY_COMPILE([
					#define _GNU_SOURCE
					#include <sys/resource.h>

					void Wrapper_setrlimit(int limit, const struct rlimit *rlp)
					{
					  (void)setrlimit(limit, rlp);
					}],
				       [],
				       [ensc_cv_sys_broken_rlimit_protos=no],
				       [ensc_cv_sys_broken_rlimit_protos=yes])
			AC_LANG_POP(C)])

	if test x"${ensc_cv_sys_broken_rlimit_protos}" = xyes; then
		AC_DEFINE(HAVE_BROKEN_RLIMIT_PROTOTYPES, 1,
			  [Define if *etrlimit() function are having broken prototypes])
	fi
])


AC_DEFUN([ENSC_MODERN_COMPILER_CHECK],
[
	AC_CACHE_CHECK([whether $CC is modern],
		       [ensc_cv_sys_modern_cc],
		       [AC_LANG_PUSH(C)
			old_CFLAGS="$CFLAGS"
			CFLAGS="-Werror -pedantic"
		AC_TRY_COMPILE([inline static void ensc_foo() {}
		       ],
				       [],
				       [ensc_cv_sys_modern_cc=yes],
			       [ensc_cv_sys_modern_cc=no])
			AC_LANG_POP(C)
			CFLAGS="$old_CFLAGS"
		       ])


	if test x"${ensc_cv_sys_modern_cc}" = xyes; then
		AC_DEFINE([HAVE_MODERN_COMPILER], 1,
			  [Define if used compiler is modern])
	fi
])
