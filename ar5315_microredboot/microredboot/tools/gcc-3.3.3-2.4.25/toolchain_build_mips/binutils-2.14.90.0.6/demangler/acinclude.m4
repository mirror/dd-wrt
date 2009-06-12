sinclude(../libtool.m4)
dnl The lines below arrange for aclocal not to bring libtool.m4
dnl AC_PROG_LIBTOOL into aclocal.m4, while still arranging for automake
dnl to add a definition of LIBTOOL to Makefile.in.
ifelse(yes,no,[
AC_DEFUN([AC_PROG_LIBTOOL],)
AC_DEFUN([AM_PROG_LIBTOOL],)
AC_SUBST(LIBTOOL)
])

# Newer versions of autoconf add an underscore to these functions.
# Prevent future problems ...
ifdef([AC_PROG_CXX_G],[],[define([AC_PROG_CXX_G],defn([_AC_PROG_CXX_G]))])
ifdef([AC_PROG_CXX_GNU],[],[define([AC_PROG_CXX_GNU],defn([_AC_PROG_CXX_GNU]))])

# We can't just call AC_PROG_CXX directly because g++ will try to
# link in libstdc++.
AC_DEFUN(libdemangler_AC_PROG_CXX,
[AC_BEFORE([$0], [AC_PROG_CXXCPP])dnl
dnl Fool anybody using AC_PROG_CXX.
AC_PROVIDE([AC_PROG_CXX])
AC_CHECK_PROGS(CXX, $CCC c++ g++ gcc CC cxx cc++, gcc)

AC_PROG_CXX_GNU

if test $ac_cv_prog_gxx = yes; then
  GXX=yes
  dnl Check whether -g works, even if CXXFLAGS is set, in case the package
  dnl plays around with CXXFLAGS (such as to build both debugging and
  dnl normal versions of a library), tasteless as that idea is.
  ac_test_CXXFLAGS="${CXXFLAGS+set}"
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS=
  AC_PROG_CXX_G
  if test "$ac_test_CXXFLAGS" = set; then
    CXXFLAGS="$ac_save_CXXFLAGS"
  elif test $ac_cv_prog_cxx_g = yes; then
    CXXFLAGS="-g -O2"
  else
    CXXFLAGS="-O2"
  fi
else
  GXX=
  test "${CXXFLAGS+set}" = set || CXXFLAGS="-g"
fi
])

dnl See if we can use the new demangler in C++.
AC_DEFUN(libdemangler_AC_NEW_DEMANGLER,
[AC_CACHE_CHECK(whether we can use the new demangler in C++,
		libdemangler_cv_new_demangler,
[saved_CPPFLAGS="$CPPFLAGS"
 saved_CXXFLAGS="$CXXFLAGS"
 CPPFLAGS="-I$srcdir -I$srcdir/../include -D_GLIBCXX_DEMANGLER_NOSTDCXX $CPPFLAGS"
 CXXFLAGS="-fno-rtti -fno-exceptions $CXXFLAGS"
 libdemangler_cv_new_demangler=no
 ac_compile='${CXX-g++} $CXXFLAGS $CPPFLAGS -c -I$srcdir/include $srcdir/lib/demangle.cc 1>&5'
 if { (eval echo configure:__oline__: \"$ac_compile\") 1>&5; (eval $ac_compile ) 2>&5; }; then
   ac_compile='${CC-cc} -c $CFLAGS $CPPFLAGS $srcdir/../libiberty/dyn-string.c 1>&5'
   if { (eval echo configure:__oline__: \"$ac_compile\") 1>&5; (eval $ac_compile ) 2>&5; }; then
     cat > conftest.c <<EOF
#include <stddef.h>
#include <stdio.h>
#include "demangle.h"

void
xexit (code)
    int code;
{
  exit (code);
}

char *
xmalloc (size)
    size_t size;
{
  return (char *) 0;
}

char *
xrealloc (oldmem, size)
    char *oldmem;
    size_t size;
{
  return (char *) 0;
}

int
main ()
{
  const char* p;
  p = cplus_demangle_v3 ("_Z1fA37_iPS_", 0);
  return !p;
}
EOF
     ac_compile='${CC-cc} -c $CFLAGS $CPPFLAGS conftest.c 1>&5'
     if { (eval echo configure:__oline__: \"$ac_compile\") 1>&5; (eval $ac_compile ) 2>&5; }; then
       ac_link='${CC-cc} -o conftest${ac_exeext} conftest.o demangle.o dyn-string.o $CFLAGS $CPPFLAGS $LDFLAGS $LIBS 1>&5'
       if { (eval echo configure:__oline__: \"$ac_link\") 1>&5; (eval $ac_link ) 2>&5; }; then
         libdemangler_cv_new_demangler=yes
       fi
     fi
   fi
 fi
 rm -f demangle.o dyn-string.o conftest*
 CPPFLAGS="$saved_CPPFLAGS"
 CXXFLAGS="$saved_CXXFLAGS"])
 ])
