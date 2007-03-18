dnl ===================================================================
dnl dast.m4
dnl by Ajay Tirumala <tirumala@ncsa.uiuc.edu>
dnl    and Mark Gates <mgates@nlanr.net>
dnl Copyright 1999-2002 Board of Trustees of the University of Illinois.
dnl See the main web page for full copyrights notice and disclaimer
dnl ===================================================================


dnl ===================================================================
dnl Check for IPV6 support
dnl We have avoided checking for ss_family in sockaddr_storage because
dnl linux 2.4.x versions support IPv6 but do not have this structure
dnl Just check for the presence of sockaddr_storage, sockaddr_in6 
dnl and sin6_port
dnl ===================================================================
AC_DEFUN(DAST_CHECK_IPV6, [	
    AC_TRY_COMPILE(
	[#include <sys/types.h>
    	 #include <sys/socket.h>
	 #include <netinet/in.h>
	 #include <netinet/tcp.h>
	],
      	[struct sockaddr_storage sa_union, *sa_unionp;
  	 struct sockaddr_in6 *sa;
  	 sa_unionp = &sa_union;
  	 sa = (struct sockaddr_in6 *)sa_unionp;
  	 sa->sin6_port = ntohs(5001);
        ],
    	ac_accept_ipv6=yes,
    	ac_accept_ipv6=no)
    if test "$ac_accept_ipv6" = yes ; then
    	AC_DEFINE(IPV6)
    fi	
])

dnl ===================================================================
dnl DAST_CHECK_BOOL
dnl Check for bool support. Defines bool, true, and false.


AC_DEFUN(DAST_CHECK_BOOL, [

AC_CHECK_SIZEOF(bool)
if test "$ac_cv_sizeof_bool" = 0 ; then
  AC_DEFINE(bool, int)
fi

AC_CACHE_CHECK(if true is defined, ac_cv_have_true,
  AC_TRY_COMPILE([],
    [unsigned int i = true],
  ac_cv_have_true=yes,
  ac_cv_have_true=no))

if test "$ac_cv_have_true" != yes ; then
  AC_DEFINE(true,  1)
  AC_DEFINE(false, 0)
fi

])

dnl ===================================================================
dnl DAST_CHECK_TYPE( type, includes [, default] )
dnl Check whether a type exists.
dnl If it does not exist, use the default (if given).
dnl Define HAVE_<type> if it exists or a default is given.

AC_DEFUN(DAST_CHECK_TYPE, [

AC_CACHE_CHECK(for $1, ac_cv_type_$1,
  AC_TRY_COMPILE(
    [$2],
    [$1 foo],
  ac_cv_type_$1=yes,
  ac_cv_type_$1=no)

if test "$ac_cv_type_$1" = no; then
  if test -n "$3"; then
    ac_cv_type_$1="$3"
  fi
fi)

if test "$ac_cv_type_$1" != no; then
  if test "$ac_cv_type_$1" != yes; then
    AC_DEFINE_UNQUOTED($1, $ac_cv_type_$1)
  else
    AC_DEFINE_UNQUOTED(HAVE_`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`)
  fi
fi

])

dnl ===================================================================
dnl DAST_CHECK_TYPE_SIZEOF( type, includes, sizeof [, default] )
dnl Check for the type as AC_CHECK_TYPE does. Define HAVE_<type>
dnl if type exists; don't define <type> to anything if it doesn't exist.
dnl Useful if there is no well-defined default type, such as int32_t
dnl Also allows additional include files

AC_DEFUN(DAST_CHECK_TYPE_SIZEOF, [

AC_CACHE_CHECK(for $1, ac_cv_type_$1,
  AC_TRY_COMPILE(
    [$2],
    [$1 foo],
  ac_cv_type_$1=yes,
  ac_cv_type_$1=no)

if test $ac_cv_type_$1 != yes ; then
  if test "$ac_cv_sizeof_char" = $3; then
    ac_cv_type_$1="char"
  else if test "$ac_cv_sizeof_short" = $3; then
    ac_cv_type_$1="short"
  else if test "$ac_cv_sizeof_int" = $3; then
    ac_cv_type_$1="int"
  else if test "$ac_cv_sizeof_long" = $3; then
    ac_cv_type_$1="long"
  else if test "$ac_cv_sizeof_long_long" = $3; then
    ac_cv_type_$1="long long"
  fi fi fi fi fi
fi

if test "$ac_cv_type_$1" = no; then
  if test -n "$4"; then
    ac_cv_type_$1="$4"
  fi
fi)

if test "$ac_cv_type_$1" != no; then
  if test "$ac_cv_type_$1" != yes; then
    AC_DEFINE_UNQUOTED($1, $ac_cv_type_$1)
  else
    AC_DEFINE_UNQUOTED(HAVE_`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`)
  fi
fi

])

AC_DEFUN(DAST_CHECK_TYPE_SIZEOF_UNSIGNED, [

AC_CACHE_CHECK(for $1, ac_cv_type_$1,
  AC_TRY_COMPILE(
    [$2],
    [$1 foo],
  ac_cv_type_$1=yes,
  ac_cv_type_$1=no)

if test $ac_cv_type_$1 != yes ; then
  if test "$ac_cv_sizeof_unsigned_char" = $3; then
    ac_cv_type_$1="unsigned char"
  else if test "$ac_cv_sizeof_unsigned_short" = $3; then
    ac_cv_type_$1="unsigned short"
  else if test "$ac_cv_sizeof_unsigned_int" = $3; then
    ac_cv_type_$1="unsigned int"
  else if test "$ac_cv_sizeof_unsigned_long" = $3; then
    ac_cv_type_$1="unsigned long"
  else if test "$ac_cv_sizeof_unsigned_long_long" = $3; then
    ac_cv_type_$1="unsigned long long"
  fi fi fi fi fi
fi

if test "$ac_cv_type_$1" = no; then
  if test -n "$4"; then
    ac_cv_type_$1="$4"
  fi
fi)

if test "$ac_cv_type_$1" != no; then
  if test "$ac_cv_type_$1" != yes; then
    AC_DEFINE_UNQUOTED($1, $ac_cv_type_$1)
  else
    AC_DEFINE_UNQUOTED(HAVE_`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`)
  fi
fi

])

dnl ===================================================================
dnl DAST_ASK( message, variable, default )
dnl Prompts the user for input, returning result in the variable
dnl If the variable is cached, use that as the default,
dnl otherwise use the given default.

AC_DEFUN(DAST_ASK, [

dnl ask for library path
dnl if not cached, set to a default
if test -z "$$2"; then
  $2="$3"
fi
changequote(, )dnl
echo "$1 [$$2] "
changequote([, ])dnl
read tmp
if test -n "$tmp"; then  
  $2="$tmp"
fi

])
dnl end DAST_ASK

dnl ===================================================================
dnl DAST_PROG_CC
dnl similar to AC_PROG_CC, but allow the user to input
dnl a different compiler, and set CFLAGS to -O2

AC_DEFUN(DAST_PROG_CC, [
dnl Several things can occur when looking for a compiler:
dnl   1) configure's cache exists, so use that compiler
dnl   2) $CC is defined, so use that compiler
dnl   3) ask, user gives one, so use that compiler
dnl   4) ask, none given, so let autoconf pick

OLD_CFLAGS="$CFLAGS"

if test -n "$ac_cv_prog_CC"; then
  echo "Using cached C compiler $ac_cv_prog_CC"
  echo "To change compilers, use \"make distclean\" then rerun configure"
fi

AC_PROG_CC

dnl Set our CFLAGS to avoid getting -g  debug flag.
dnl Set optimization to level 2
dnl Note: optimization is set differently for various compilers
dnl   most  use  CFLAGS="-O2"     -->  opt level 2
dnl   HPUX  uses CFLAGS="+O2 -Ae" -->  opt level 2, ANSI C
dnl   SunOS uses CFLAGS="-xO2"    -->  opt level 2
dnl For gcc, set -Wall for (most) all warnings
dnl We should use autoconf's canonical host/cpu/compiler names

if test "$ac_cv_prog_gcc" = yes ; then
  CFLAGS="$OLD_CFLAGS -Wall -O2"
else  
  if test `uname -s` = "HP-UX"; then
    CFLAGS="$OLD_CFLAGS -Ae +O2"
  elif test `uname -s` = "SunOS"; then
    CFLAGS="$OLD_CFLAGS -xO2"
  else
    CFLAGS="$OLD_CFLAGS -O2"
  fi
fi
])
dnl end DAST_PROG_CC

dnl ===================================================================
dnl DAST_PROG_CXX
dnl similar to AC_PROG_CXX, but allow the user to input
dnl a different compiler, and set CXXFLAGS to -O2

AC_DEFUN(DAST_PROG_CXX, [

dnl Several things can occur when looking for a compiler:
dnl   1) configure's cache exists, so use that compiler
dnl   2) $CC is defined, so use that compiler
dnl   3) ask, user gives one, so use that compiler
dnl   4) ask, none given, so let autoconf pick

OLD_CXXFLAGS="$CXXFLAGS"

if test -n "$ac_cv_prog_CXX"; then
  echo "Using cached C++ compiler $ac_cv_prog_CXX"
  echo "To change compilers, use \"make distclean\" then rerun configure"
fi

AC_PROG_CXX

dnl Set our CXXFLAGS to avoid getting -g  debug flag.
dnl Set optimization to level 2
dnl Note: optimization is set differently for various compilers
dnl   most  use  CXXFLAGS="-O2"     -->  opt level 2
dnl   HPUX  uses CXXFLAGS="+O2 -Ae" -->  opt level 2, ANSI C
dnl   SunOS uses CXXFLAGS="-xO2"    -->  opt level 2
dnl For gcc, set -Wall for (most) all warnings
dnl We should use autoconf's canonical host/cpu/compiler names

if test "$ac_cv_prog_gcc" = yes ; then
  CXXFLAGS="$OLD_CXXFLAGS -Wall -O2"
else  
  if test `uname -s` = "HP-UX"; then
    CXXFLAGS="$OLD_CXXFLAGS -Ae +O2"
  elif test `uname -s` = "SunOS"; then
    CXXFLAGS="$OLD_CXXFLAGS -xO2"
  else
    CXXFLAGS="$OLD_CXXFLAGS -O2"
  fi
fi
])
dnl end DAST_PROG_CXX

dnl ===================================================================
dnl DAST_CHECK_LIB( library, function )
dnl Exactly the same as the corresponding AC_ macro, but
dnl **always** insert the #ifdef __cplusplus code. We could
dnl unknowingly be using a C++ compiler as our C compiler.

dnl DAST_CHECK_LIB(LIBRARY, FUNCTION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND
dnl              [, OTHER-LIBRARIES]]])
AC_DEFUN(DAST_CHECK_LIB,
[AC_MSG_CHECKING([for $2 in -l$1])
dnl Use a cache variable name containing both the library and function name,
dnl because the test really is for library $1 defining function $2, not
dnl just for library $1.  Separate tests with the same $1 and different $2s
dnl may have different results.
ac_lib_var=`echo $1['_']$2 | sed 'y%./+-%__p_%'`
AC_CACHE_VAL(ac_cv_lib_$ac_lib_var,
[ac_save_LIBS="$LIBS"
LIBS="-l$1 $5 $LIBS"
AC_TRY_LINK(dnl
ifelse([$2], [main], , dnl Avoid conflicting decl of main.
[/* Override any gcc2 internal prototype to avoid an error.  */
#ifdef __cplusplus
extern "C"
#endif
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char $2();
]),
	    [$2()],
	    eval "ac_cv_lib_$ac_lib_var=yes",
	    eval "ac_cv_lib_$ac_lib_var=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test \"`echo '$ac_cv_lib_'$ac_lib_var`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$3], ,
[changequote(, )dnl
  ac_tr_lib=HAVE_LIB`echo $1 | sed -e 's/[^a-zA-Z0-9_]/_/g' \
    -e 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/'`
changequote([, ])dnl
  AC_DEFINE_UNQUOTED($ac_tr_lib)
  LIBS="-l$1 $LIBS"
], [$3])
else
  AC_MSG_RESULT(no)
ifelse([$4], , , [$4
])dnl
fi
])
dnl end DAST_CHECK_LIB

dnl ===================================================================
dnl DAST_CHECK_NO_LIB( library, function )
dnl checks first for the function WITHOUT the library,
dnl then use DAST_CHECK_LIB if not found.

AC_DEFUN(DAST_CHECK_NO_LIB, [

AC_MSG_CHECKING([for $2 without -l$1])
ac_lib_var=`echo $1['_']$2 | sed 'y%./+-%__p_%'`
AC_CACHE_VAL(ac_cv_no_lib_$ac_lib_var, [

dnl This section is almost straight from AC_CHECK_LIB (change result variable)
dnl and **always** insert the #ifdef __cplusplus code. We could
dnl unknowingly be using a C++ compiler as our C compiler.

AC_TRY_LINK(dnl
ifelse([$2], [main], , dnl Avoid conflicting decl of main.
[/* Override any gcc2 internal prototype to avoid an error.  */
#ifdef __cplusplus
extern "C"
#endif
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char $2();
]),
	    [$2()],
	    eval "ac_cv_no_lib_$ac_lib_var=yes",
	    eval "ac_cv_no_lib_$ac_lib_var=no")
])dnl

dnl If we couldn't link without the library, see if we can with it
result=`eval echo '$ac_cv_no_lib_'$ac_lib_var`
AC_MSG_RESULT($result)
if test "$result" = no ; then
  AC_CHECK_LIB($1, $2)
fi

])
dnl end DAST_CHECK_NO_LIB

dnl ===================================================================
dnl DAST_CHECK_PTHREAD
dnl look for the pthreads library. If found, define HAVE_PTHREAD
dnl and add -D_REENTRANT to the compiler flags.

AC_DEFUN(DAST_CHECK_PTHREAD, [

DAST_CHECK_LIB(pthreads, pthread_create)
if test "$ac_cv_lib_pthreads_pthread_create" != yes ; then
  DAST_CHECK_LIB(pthread, pthread_create)
fi
if test "$ac_cv_lib_pthread_pthread_create"  = yes || \
   test "$ac_cv_lib_pthreads_pthread_create" = yes; then
  AC_DEFINE(HAVE_POSIX_THREAD)
  AC_DEFINE(_REENTRANT)
fi

])
dnl end DAST_CHECK_PTHREAD

dnl ===================================================================
dnl DAST_OUTPUT( files )
dnl The same as AC_OUTPUT, but also chmod's the files the be read only.
dnl That way no one accidentally modifies configure output files.

AC_DEFUN(DAST_OUTPUT, [
ifdef([AC_LIST_HEADER],
      [AC_OUTPUT( $1, [chmod a-w $1 AC_LIST_HEADER] )],
      [AC_OUTPUT( $1, [chmod a-w $1] )])
])

dnl ===================================================================
dnl DAST_CHECK_FUNC( ... )
dnl DAST_CHECK_FUNCS( ... )
dnl DAST_REPLACE_FUNCS( ... )
dnl Exactly the same as the corresponding AC_ macros, but
dnl **always** insert the #ifdef __cplusplus code. We could
dnl unknowingly be using a C++ compiler as our C compiler.

dnl DAST_CHECK_FUNC(FUNCTION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN(DAST_CHECK_FUNC,
[AC_MSG_CHECKING([for $1])
AC_CACHE_VAL(ac_cv_func_$1,
[AC_TRY_LINK(
dnl Don't include <ctype.h> because on OSF/1 3.0 it includes <sys/types.h>
dnl which includes <sys/select.h> which contains a prototype for
dnl select.  Similarly for bzero.
[/* System header to define __stub macros and hopefully few prototypes,
    which can conflict with char $1(); below.  */
#include <assert.h>
/* Override any gcc2 internal prototype to avoid an error.  */
#ifdef __cplusplus
extern "C"
#endif
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char $1();
], [
/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_$1) || defined (__stub___$1)
choke me
#else
$1();
#endif
], eval "ac_cv_func_$1=yes", eval "ac_cv_func_$1=no")])
if eval "test \"`echo '$ac_cv_func_'$1`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$2], , :, [$2])
else
  AC_MSG_RESULT(no)
ifelse([$3], , , [$3
])dnl
fi
])

dnl DAST_CHECK_FUNCS(FUNCTION... [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN(DAST_CHECK_FUNCS,
[for ac_func in $1
do
DAST_CHECK_FUNC($ac_func,
[changequote(, )dnl
  ac_tr_func=HAVE_`echo $ac_func | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
changequote([, ])dnl
  AC_DEFINE_UNQUOTED($ac_tr_func) $2], $3)dnl
done
])

dnl DAST_REPLACE_FUNCS(FUNCTION...)
AC_DEFUN(DAST_REPLACE_FUNCS,
[DAST_CHECK_FUNCS([$1], , [LIBOBJS="$LIBOBJS ${ac_func}.o"])
AC_SUBST(LIBOBJS)dnl
])
