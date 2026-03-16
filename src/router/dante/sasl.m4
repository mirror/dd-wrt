dnl sasl.m4 -- find compiler and linker flags for SASL
dnl Based on patch from Markus Moeller (markus_moeller at compuserve.com)

dnl default prefix for sasl headers/libs
sasldir=/usr
AC_ARG_WITH(sasl,
 [  --without-sasl          disable sasl support @<:@default=detect@:>@],
 [SASL=$withval])

AC_ARG_WITH(sasl-path,
 [  --with-sasl-path=PATH   specify sasl path],
 [sasldir=$withval])

if test x"$SASL" != xno; then
   unset saslfail

   if test x"$sasldir" != xno; then
      ac_sasl_cflags=-I$sasldir/include
      ac_sasl_libs=-L$sasldir/lib
   else
      ac_sasl_cflags=
      ac_sasl_libs=
   fi

   dnl any cflags values obtained from krb5-config?
   if test x"${ac_sasl_cflags}" != x; then
      CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }$ac_sasl_cflags"
   fi

   dnl extract -L flags
   if test x"${ac_sasl_libs}" != x; then
      NPATH=`echo $ac_sasl_libs | xargs -n1 | grep -E '^-L' | xargs echo`
      LDFLAGS="${LDFLAGS}${LDFLAGS:+ }$NPATH"
   fi

   dnl look for gssapi headers
   AC_CHECK_HEADERS(sasl.h sasl/sasl.h)

   case `uname` in
      Darwin) ac_lib_sav="$LIBS"
              AC_CHECK_LIB(sasl2, main)
	      if test x"$LIBS" != x; then
                 LIBS="$ac_lib_sav"
              fi
              if test x"${ac_cv_lib_sasl2_main}" = yes; then
                 AC_DEFINE(HAVE_SASL_DARWIN, 1, [Define to 1 if Mac Darwin without sasl.h])
              fi
              ;;
   esac

   if test x"${ac_cv_header_sasl_h}" = xyes ||
      test x"${ac_cv_header_sasl_sasl_h}" = xyes ||
      test x"${ac_cv_lib_sasl2_main}" = xyes; then
      unset no_sasl
      AC_DEFINE(HAVE_SASL, 1, [Have SASL support])
   fi
fi
