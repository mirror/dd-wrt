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

AC_DEFUN([_ENSC_DIETLIBC_C99],
[
	AH_TEMPLATE([ENSC_DIETLIBC_C99], [Define to 1 if dietlibc supports C99])

	AC_CACHE_CHECK([whether dietlibc supports C99], [ensc_cv_c_dietlibc_c99],
	[
		_ensc_dietlibc_c99_old_CFLAGS=$CFLAGS
		_ensc_dietlibc_c99_old_CC=$CC

		CFLAGS="-std=c99"
		CC="${DIET:-diet} $CC"

		AC_LANG_PUSH(C)
		AC_COMPILE_IFELSE([AC_LANG_SOURCE([/* */])],[
			AC_COMPILE_IFELSE([AC_LANG_SOURCE([
				#include <stdint.h>
				#include <sys/cdefs.h>
				#if defined(inline)
				#  error 'inline' badly defined
				#endif
				volatile uint64_t	a;
			])],
			[ensc_cv_c_dietlibc_c99=yes],
			[ensc_cv_c_dietlibc_c99=no])],
			[ensc_cv_c_dietlibc_c99='skipped (compiler does not support C99)'])
		AC_LANG_POP

		CC=$_ensc_dietlibc_c99_old_CC
		CFLAGS=$_ensc_dietlibc_c99_old_CFLAGS
	])

	if test x"$ensc_cv_c_dietlibc_c99" = xyes; then
		AC_DEFINE(ENSC_DIETLIBC_C99,1)
	fi
])

dnl Usage: ENSC_ENABLE_DIETLIBC(<conditional>[,<min-version>])
dnl        <conditional> ... automake-conditional which will be set when
dnl                          dietlibc shall be enabled
dnl        provides:
dnl        * $ENSC_VERSION_DIETLIBC_NUM and
dnl        * $ENSC_VERSION_DIETLIBC

AC_DEFUN([ENSC_ENABLE_DIETLIBC],
[
	AC_MSG_CHECKING([whether to enable dietlibc])

	AC_ARG_VAR(DIET,      [The 'diet' wrapper (default: diet)])
	AC_ARG_VAR(DIETFLAGS, [Flags passed to the 'diet' wrapper (default: -O)])

	: ${DIET:=diet}
	: ${DIETFLAGS=-Os}

	AC_ARG_ENABLE([dietlibc],
		      [AC_HELP_STRING([--disable-dietlibc],
				      [do not use dietlibc (default: use dietlibc)])],
		      [case "$enableval" in
			  (yes)	use_dietlibc=forced;;
			  (no)	use_dietlibc=forced_no;;
			  (*)	AC_MSG_ERROR(['$enableval' is not a valid value for --enable-dietlibc]);;
		       esac],
		      [which "$DIET" >/dev/null 2>/dev/null && use_dietlibc=detected || use_dietlibc=detected_no])

	if test x"$use_dietlibc" = xforced -o x"$use_dietlibc" = xdetected; then
	    _dietlibc_ver=$($DIET -v 2>&1 | sed '1p;d')
	    _dietlibc_ver=${_dietlibc_ver##*diet version }
	    _dietlibc_ver=${_dietlibc_ver##*dietlibc-}
	    _dietlibc_ver_maj=${_dietlibc_ver%%.*}
	    _dietlibc_ver_min=${_dietlibc_ver##*.}
	    _dietlibc_ver_min=${_dietlibc_ver_min%%[[!0-9]]*}

	    ENSC_VERSION_DIETLIBC=$_dietlibc_ver_maj.$_dietlibc_ver_min

	    let _dietlibc_ver=_dietlibc_ver_maj*1000+_dietlibc_ver_min 2>/dev/null || _dietlibc_ver=0
	else
	    ENSC_VERSION_DIETLIBC=
	    _dietlibc_ver=-1
	fi

	if test "$use_dietlibc" = detected -a -n '$2'; then
	    _dietlibc_cmp=$2
	    _dietlibc_cmp_maj=${_dietlibc_cmp%%.*}
	    _dietlibc_cmp_min=${_dietlibc_cmp##*.}

	    let _dietlibc_cmp=_dietlibc_cmp_maj*1000+_dietlibc_cmp_min

	    test $_dietlibc_ver -ge $_dietlibc_cmp || use_dietlibc=detected_old
	fi

	ENSC_VERSION_DIETLIBC_NUM=$_dietlibc_ver
	ensc_have_dietlibc=no

	case x"$use_dietlibc" in
	    xdetected)
		AM_CONDITIONAL($1, true)
		AC_MSG_RESULT([yes (autodetected, $ENSC_VERSION_DIETLIBC)])
		ensc_have_dietlibc=yes
		;;
	    xforced)
		AM_CONDITIONAL($1, true)
		AC_MSG_RESULT([yes (forced)])
		ensc_have_dietlibc=yes
		;;
	    xdetected_no)
		AM_CONDITIONAL($1, false)
		AC_MSG_RESULT([no (detected)])
		;;
	    xdetected_old)
		AM_CONDITIONAL($1, false)
		AC_MSG_RESULT([no (too old; $2+ required, $ENSC_VERSION_DIETLIBC found)])
		;;
	    xforced_no)
		AM_CONDITIONAL($1, false)
		AC_MSG_RESULT([no (forced)])
		;;
	    *)
		AC_MSG_ERROR([internal error, use_dietlibc was "$use_dietlibc"])
		;;
	esac

	if test x"$ensc_have_dietlibc" != xno; then
		_ENSC_DIETLIBC_C99
	fi
])


dnl Usage: ENSC_DIETLIBC_SANITYCHECK
AC_DEFUN([ENSC_DIETLIBC_SANITYCHECK],
[
	AC_REQUIRE([AC_CANONICAL_HOST])
	AC_REQUIRE([ENSC_ENABLE_DIETLIBC])

	if test "$host_cpu" = x86_64 -a $ENSC_VERSION_DIETLIBC_NUM -le 0027; then
		AC_MSG_WARN([***                                                             ***])
		AC_MSG_WARN([*** dietlibc<=0.27 is known to be broken for x86_64 systems     ***])
		AC_MSG_WARN([*** please make sure that at least the environ.S fix is applied ***])
		AC_MSG_WARN([*** and lib/__nice.c added                                      ***])
		AC_MSG_WARN([***                                                             ***])
	fi
])
