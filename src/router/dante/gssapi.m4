dnl gssapi.m4 -- find compiler and linker flags for GSSAPI
dnl Based on patch from Markus Moeller (markus_moeller at compuserve.com)

nogssDLIBDEPS=$DLIBDEPS
nogssSOCKDDEPS=$SOCKDDEPS
nogssCPPFLAGS=$CPPFLAGS
nogssLDFLAGS=$LDFLAGS
nogssCFLAGS=$CFLAGS
nogssLIBS=$LIBS

dnl default prefix for gssapi headers/libs
unset gssdir
unset krb5confpath

unset gssapi_default
case $host in
    #openbsd libthread is buggy; disable gssapi by default (needs pthreads)
    *-*-openbsd*)
	gssapi_default=no
	AC_DEFINE(HAVE_THREADS_EINTR_PROBLEMS, 1, [threads unstable platform])
	;;
esac

unset GSSAPI
AC_ARG_WITH(gssapi-path,
[  --with-gssapi-path=PATH specify gssapi path],
[if test -e "$withval"; then
     gssdir=$withval
     #act as if --with-gssapi is specified if path is given
     GSSAPI="yes"
 else
     AC_MSG_WARN([invalid gssapi-path: $withval])
 fi])

AC_ARG_WITH(gssapi,
[  --without-gssapi        disable gssapi support],
[GSSAPI=$withval],
[if test x"${gssapi_default}" = xno -a x"$GSSAPI" != xyes; then
     GSSAPI="no"
     AC_MSG_WARN([gssapi disabled by default on this platform])
 fi])

AC_ARG_WITH(krb5-config,
[  --with-krb5-config=PATH specify path to krb5-config @<:@default=detect@:>@],
[if test x"$withval" != xno; then
     krb5confpath=$withval
 fi])

if test x"$GSSAPI" != xno; then
   #attempt to create build environment based on prefix
   if test x"$gssdir" != x; then
      if test x"$krb5confpath" = x -a -x "$gssdir/bin/krb5-config"; then
            krb5confpath=$gssdir/bin/krb5-config
      fi
   else
      #set /usr as default gssdir
      case $host in
          *-*-aix*)
	      gssdir=/usr/krb5
	      ;;
	  *)
	      gssdir=/usr
	      ;;
      esac
   fi

   unset krb5fail
   if test x"$krb5confpath" != xno; then
       if test x"$krb5confpath" != x; then
           if ! test -x "$krb5confpath"; then
	       AC_MSG_WARN([krb5-config '$krb5confpath' not executable, ignoring])
           else
               ac_gssapi_cflags=`$krb5confpath --cflags gssapi 2>/dev/null`
	       if test $? != 0; then
		   krb5fail=t
	       fi
	       ac_gssapi_libs=`$krb5confpath --libs gssapi 2>/dev/null`
	       if test $? != 0; then
		   krb5fail=t
	       fi
           fi
       else
           AC_CHECK_PROG(ac_krb5_config, krb5-config, yes, no)
           if test x"${ac_krb5_config}" = xyes; then
	       ac_gssapi_cflags=`krb5-config --cflags gssapi 2>/dev/null`
	       if test $? != 0; then
		   krb5fail=t
	       fi
	       ac_gssapi_libs=`krb5-config --libs gssapi 2>/dev/null`
	       if test $? != 0; then
		   krb5fail=t
	       fi
           fi
       fi
   fi
   dnl working krb5-config?
   if test x"$krb5fail" = xt; then
       ac_gssapi_cflags=
       ac_gssapi_libs=
   fi

   dnl nothing from krb5-config, but gssdir specified?
   if test x"${ac_gssapi_cflags}" = x -a x"${ac_gssapi_libs}" = x -a \
           x"$gssdir" != x; then
       dnl attempt to construct environment manually
       case $host in
           *-*-aix*)
	       CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-I/usr/include"
	       ;;
	   *)
	       CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-I$gssdir/include"
	       ;;
       esac

       LDFLAGS="${LDFLAGS}${LDFLAGS:+ }-L$gssdir/lib"
       #includes under kerberosV dir on OpenBSD
       if test -d "$gssdir/include/kerberosV"; then
		  CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-I${gssdir}/include/kerberosV"
       fi
   fi

   dnl any cflags values obtained from krb5-config?
   if test x"${ac_gssapi_cflags}" != x; then
       CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }$ac_gssapi_cflags"
   fi

   dnl any libs obtained from krb5-config?
   if test x"${ac_gssapi_libs}" != x; then
       dnl add as dependency for sockd
       SOCKDDEPS="${SOCKDDEPS}${SOCKDDEPS:+ }$ac_gssapi_libs"

       #need to use -lpthread rather than -pthread for dlib because
       #the first will not cause -lpthread to be linked in
       #(and the application might not be linked with -lphread).
       #applications should use -pthread so keep this for SOCKDDEPS
       NDEPS=`echo $ac_gssapi_libs | sed -e 's/-pthread/-lpthread/g'`
       dnl add as dependency for libdsocks/libsocks
       DLIBDEPS="${DLIBDEPS}${DLIBDEPS:+ }$NDEPS"
   fi

   dnl look for gssapi headers
   AC_CHECK_HEADERS(gssapi.h gssapi/gssapi.h gssapi/gssapi_ext.h)
   dnl gssapi_krb5.h might depend on gssapi.h
   AC_CHECK_HEADERS([gssapi/gssapi_krb5.h], [], [], [
#if HAVE_GSSAPI_H
#include <gssapi.h>
#elif HAVE_GSSAPI_GSSAPI_H
#include <gssapi/gssapi.h>
#endif /* HAVE_GSSAPI_H */
])
   AC_CHECK_HEADERS(gssapi/gssapi_generic.h)

   dnl look for libs
   if test x"${ac_gssapi_libs}" != x; then
       _libsonly=`echo $ac_gssapi_libs | xargs -n1 | grep -E '^-l' | xargs echo`
       _optsonly=`echo $ac_gssapi_libs | xargs -n1 | grep -E -v '^-l' | xargs echo`
       LIBS="${LIBS}${LIBS:+ }${_libsonly}"
       LDFLAGS="${LDFLAGS}${LDFLAGS:+ }${_optsonly}"

       case $host in
	   *-*-freebsd7*)
	          #skip check, causes problems
	       ;;
	   *)
	       #wanted in case of Heimdal on FreeBSD, if the function
	       #is missing in libgssapi. not wanted if not needed for
	       #heimdal (potentially causes linking problems if
	       #multiple kerberos implementations are installed on a
	       #machine).
	       AC_SEARCH_LIBS([gsskrb5_register_acceptor_identity], [gssapi gssapi_krb5])
	       ;;
       esac
   else
       oLIBS=$LIBS
       LIBS=""

       #AC_CHECK_LIB(crypto, main) #XXX only very old mit kerberos
       AC_CHECK_LIB(des, main)
       AC_CHECK_LIB(crypt, main)
       AC_CHECK_LIB(roken, main)

       AC_CHECK_LIB(com_err, main)
       AC_CHECK_LIB(des425, main)
       AC_CHECK_LIB(k5crypto, main)

       AC_CHECK_LIB(pthread, main)

       AC_CHECK_LIB(asn1, main)
       AC_CHECK_LIB(krb5, main)
       AC_CHECK_LIB(gssapi_krb5, main)

       AC_CHECK_LIB(gssapi, main)

       AC_CHECK_LIB(gss, main)

       AC_CHECK_LIB(ksvc, main)

       dnl add as dependency for libdsocks
       if test x"${LIBS}" != x; then
	   DLIBDEPS="${DLIBDEPS}${DLIBDEPS:+ }$LIBS"
	   SOCKDDEPS="${SOCKDDEPS}${SOCKDDEPS:+ }$LIBS"
       fi
       LIBS="${oLIBS}${oLIBS:+ }$LIBS"
   fi


   AC_CHECK_FUNCS(gss_map_name_to_any,
      [AC_DEFINE(HAVE_GSS_MAP_ANY_TO_ANY, 1,
       [Define to 1 if you have gss_map_name_to_any])])
   AC_CHECK_FUNCS(gsskrb5_extract_authz_data_from_sec_context,
      [AC_DEFINE(HAVE_GSSKRB5_EXTRACT_AUTHZ_DATA_FROM_SEC_CONTEXT, 1,
       [Define to 1 if you have gsskrb5_extract_authz_data_from_sec_context])])

   unset have_heimdal
   AC_MSG_CHECKING([for heimdal])
   AC_TRY_LINK([
#include <stdio.h>
#include "krb5.h"], [printf("%s\n", heimdal_version);],
   [AC_DEFINE(HAVE_HEIMDAL_KERBEROS, 1, [Heimdal kerberos implementation])
    AC_MSG_RESULT(yes)
    have_heimdal=t],
   [AC_MSG_RESULT(no)])

   if test x"${have_heimdal}" = x -a x"$krb5confpath" != x; then
       heimdalout="`$krb5confpath --version 2>/dev/null | grep -i heimdal`"
       if test $? -eq 0; then
          have_heimdal=t
          AC_DEFINE(HAVE_HEIMDAL_KERBEROS, 1, [Heimdal kerberos implementation])
       fi
   fi

   dnl do compile check
   AC_MSG_CHECKING([for working gssapi])
   AC_TRY_RUN([
#if HAVE_GSSAPI_H
#include <gssapi.h>
#elif HAVE_GSSAPI_GSSAPI_H
#include <gssapi/gssapi.h>
#endif /* HAVE_GSSAPI_H */

#if !HAVE_HEIMDAL_KERBEROS
#if HAVE_GSSAPI_GSSAPI_EXT_H
#include <gssapi/gssapi_ext.h>
#endif /* HAVE_GSSAPI_GSSAPI_EXT_H */
#if HAVE_GSSAPI_GSSAPI_KRB5_H
#include <gssapi/gssapi_krb5.h>
#endif /* HAVE_GSSAPI_GSSAPI_KRB5_H */
#if HAVE_GSSAPI_GSSAPI_GENERIC_H
#include <gssapi/gssapi_generic.h>
#endif /* HAVE_GSSAPI_GSSAPI_GENERIC_H */
#endif /* HAVE_HEIMDAL_KERBEROS */

int
main(void)
{
    OM_uint32 val;
    gss_OID_set set;

    gss_create_empty_oid_set(&val, &set);

    return 0;
}
], [unset no_gssapi
    AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)],
   [dnl assume no when cross-compiling
    AC_MSG_RESULT(assuming no)])

   for file in gssapi.h gssapi/gssapi.h gssapi/gssapi_generic.h; do
       AC_EGREP_HEADER(gss_nt_service_name, [$file],
           break,
           [AC_EGREP_HEADER(gss_nt_service_name, $file,
                            GSS_C_NT_HOSTBASED_SERVICE,
		            [AC_DEFINE(gss_nt_service_name,
			     GSS_C_NT_HOSTBASED_SERVICE,
                             [gss_nt_service_name replacement])
                              break;])])
   done
fi

#restore build environment if not using gssapi
if test x"${no_gssapi}" = xt; then
    DLIBDEPS=$nogssDLIBDEPS
    SOCKDDEPS=$nogssSOCKDDEPS
    CPPFLAGS=$nogssCPPFLAGS
    LDFLAGS=$nogssLDFLAGS
    CFLAGS=$nogssCFLAGS
    LIBS=$nogssLIBS
else
    #include libs as dependencies as needed in subdirs
    LIBS="${LIBS}${LIBS:+ }$nogssLIBS"

    AC_DEFINE(HAVE_GSSAPI, 1, [GSSAPI support])
fi
