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

AC_DEFUN([__ENSC_CHECK_WARNFLAGS],
[
	warn_flags="-Werror -W"
	AC_MSG_CHECKING([whether the $1-compiler accepts ${warn_flags}])
	AC_LANG_PUSH($1)
	old_CFLAGS="${$3}"
	$3="$warn_flags"
	AC_TRY_COMPILE([inline static void f(){}],
		       [],
		       [ensc_sys_compilerwarnflags_$2=${warn_flags}],
		       [ensc_sys_compilerwarnflags_$2=])
	AC_LANG_POP($1)
	$3="$old_CFLAGS"

	if test x"${ensc_sys_compilerwarnflags_$2}" = x; then
		AC_MSG_RESULT([no])
	else
		AC_MSG_RESULT([yes])
	fi
])

AC_DEFUN([__ENSC_CHECK_WARNFLAGS_C],
[
	__ENSC_CHECK_WARNFLAGS(C, C, CFLAGS)
])

AC_DEFUN([__ENSC_CHECK_WARNFLAGS_CXX],
[
	__ENSC_CHECK_WARNFLAGS(C++, CXX, CXXFLAGS)
])


# --------------------------------------------------------------------------
# Check whether the C++ compiler accepts a certain flag
# If it does it adds the flag to CXXFLAGS
# If it does not then it returns an error to lf_ok
# Usage:
#   ENSC_CHECK_CXX_FLAG(-flag1 -flag2 -flag3 ...)
# -------------------------------------------------------------------------

AC_DEFUN([ENSC_CHECK_CXX_FLAG],
[
	AC_REQUIRE([__ENSC_CHECK_WARNFLAGS_CXX])

  echo 'void f(){}' > conftest.cc
  for i in $1
  do
    AC_MSG_CHECKING([whether $CXX accepts $i])
    if test -z "`${CXX} ${ensc_sys_compilerwarnflags_CXX} $i -c conftest.cc 2>&1`"
    then
      CXXFLAGS="${CXXFLAGS} $i"
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  done
  rm -f conftest.cc conftest.o
])

# --------------------------------------------------------------------------
# Check whether the C compiler accepts a certain flag
# If it does it adds the flag to CFLAGS
# If it does not then it returns an error to lf_ok
# Usage:
#  ENSC_CHECK_CC_FLAG(-flag1 -flag2 -flag3 ...)
# -------------------------------------------------------------------------

AC_DEFUN([ENSC_CHECK_CC_FLAG],
[
	AC_REQUIRE([__ENSC_CHECK_WARNFLAGS_C])

echo 'void f(){}' > conftest.c
  for i in $1
  do
    AC_MSG_CHECKING([whether $CC accepts $i])
    if test -z "`${CC} ${ensc_sys_compilerwarnflags_C} $i -c conftest.c 2>&1`"
    then
      CFLAGS="${CFLAGS} $i"
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  done
  rm -f conftest.c conftest.o
])

AC_DEFUN([ENSC_CHECK_DEFAULT_FLAG],
[
	if test x"${ensc_sys_default_flag}" = x; then
		ENSC_CHECK_CC_FLAG([-fmessage-length=0])
		ENSC_CHECK_CXX_FLAG([-fmessage-length=0])

		ensc_sys_default_flag=set
	fi
])
