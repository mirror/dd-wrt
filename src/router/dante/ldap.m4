dnl ldap.m4 -- find compiler and linker flags for LDAP
dnl Based on patch from Markus Moeller (markus_moeller at compuserve.com)

dnl default prefix for ldap headers/libs
ldapdir=/usr
AC_ARG_WITH(ldap,
 [  --without-ldap          disable ldap support @<:@default=detect@:>@],
 [LDAP=$withval])

AC_ARG_WITH(ldap-path,
 [  --with-ldap-path=PATH   specify ldap path],
 [ldapdir=$withval])

if test x"$LDAP" != xno; then
   unset ldapfail

   if test x"$ldapdir" != xno; then
      ac_ldap_cflags=-I$ldapdir/include
      ac_ldap_libs=-L$ldapdir/lib
   else
      ac_ldap_cflags=
      ac_ldap_libs=
   fi

   dnl any cflags values obtained from krb5-config?
   if test x"${ac_ldap_cflags}" != x; then
      CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }$ac_ldap_cflags"
   fi

   dnl extract -L flags
   if test x"${ac_ldap_libs}" != x; then
      NPATH=`echo $ac_ldap_libs | xargs -n1 | grep -E '^-L' | xargs echo`
      LDFLAGS="${LDFLAGS}${LDFLAGS:+ }$NPATH"
   fi

   dnl need res_search, found in libresolv on e.g., Linux Fedora 13
   AC_CHECK_LIB(resolv, main)

   dnl look for ldap headers
   AC_CHECK_HEADERS(ldap.h)
   AC_CHECK_HEADERS(mozldap/ldap.h)

   AC_CHECK_LIB(ldap, main)
   if test x"${ac_cv_lib_ldap_main}" != xyes; then
     AC_CHECK_LIB(pthread, main)
     AC_CHECK_LIB(ldap60, main)
     AC_CHECK_LIB(prldap60, main)
     AC_CHECK_LIB(ssldap60, main)
   fi

   dnl do compile check
   AC_MSG_CHECKING([for working ldap])
   AC_TRY_RUN([
#if HAVE_LDAP_H
#include <ldap.h>
#elif HAVE_MOZLDAP_LDAP_H
#include <mozldap/ldap.h>
#endif

int
main(void)
{
       char *host = "";
#ifdef LDAP_DEPRECATED
       int port;

       ldap_init(host, port);
#else
       LDAP *ld;
       ldap_initialize(&ld, host);
#endif /* LDAP_DEPRECATED */

       return 0;
}], [unset no_ldap
   AC_MSG_RESULT(yes)],
   AC_MSG_RESULT(no))

   unset ldap_openldap
   AC_MSG_CHECKING([for OpenLDAP])
   AC_TRY_RUN([
#if HAVE_LDAP_H
#include <ldap.h>
#endif
#include <string.h>

int
main(void)
{
   return strcmp(LDAP_VENDOR_NAME, "OpenLDAP");
}
], [AC_DEFINE(HAVE_OPENLDAP, 1, [OpenLDAP support])
   AC_MSG_RESULT(yes)
   ldap_openldap=t],
   AC_MSG_RESULT(no))

   AC_MSG_CHECKING([for Sun LDAP SDK])
   AC_TRY_RUN([
#if HAVE_LDAP_H
#include <ldap.h>
#endif
#include <string.h>

int
main(void)
{
   return strcmp(LDAP_VENDOR_NAME, "Sun Microsystems Inc.");
}], [AC_DEFINE(HAVE_SUN_LDAP_SDK, 1, [Sun LDAP SDK support])
   AC_MSG_RESULT(yes)],
   AC_MSG_RESULT(no))

   AC_MSG_CHECKING([for Mozilla LDAP SDK])
   AC_TRY_RUN([
#if HAVE_LDAP_H
#include <ldap.h>
#elif HAVE_MOZLDAP_LDAP_H
#include <mozldap/ldap.h>
#endif
#include <string.h>

int
main(void)
{
   return strcmp(LDAP_VENDOR_NAME, "mozilla.org");
}], [AC_DEFINE(HAVE_MOZILLA_LDAP_SDK, 1, [Mozilla LDAP SDK support])
   AC_MSG_RESULT(yes)],
   AC_MSG_RESULT(no))

dnl
dnl Implementation dependent checks
dnl
   if test x"${ldap_openldap}" != x; then
      AC_CHECK_HEADERS(lber.h)
      AC_CHECK_LIB(lber, main)
   fi

dnl
dnl Check for LDAP_REBINDPROC_CALLBACK
dnl
   if test x"${ac_cv_header_ldap_h}" = xyes; then
      AC_EGREP_HEADER(LDAP_REBINDPROC_CALLBACK, ldap.h,
         AC_DEFINE(HAVE_LDAP_REBINDPROC_CALLBACK, 1, [Define to 1 if you have LDAP_REBINDPROC_CALLBACK]))
   elif test x"${ac_cv_header_mozldap_ldap_h}" = xyes; then
      AC_EGREP_HEADER(LDAP_REBINDPROC_CALLBACK, mozldap/ldap.h,
         AC_DEFINE(HAVE_LDAP_REBINDPROC_CALLBACK, 1, [Define to 1 if you have LDAP_REBINDPROC_CALLBACK]))
   else
      AC_MSG_CHECKING([for LDAP_REBINDPROC_CALLBACK])
      AC_DEFINE(HAVE_LDAP_REBINDPROC_CALLBACK, 0, [Define to 1 if you have LDAP_REBINDPROC_CALLBACK])
      AC_MSG_RESULT(no)
   fi
dnl
dnl Check for LDAP_REBIND_PROC
dnl
   if test x"${ac_cv_header_ldap_h}" = xyes; then
      AC_EGREP_HEADER(LDAP_REBIND_PROC, ldap.h,
         AC_DEFINE(HAVE_LDAP_REBIND_PROC, 1, [Define to 1 if you have LDAP_REBIND_PROC]))
   elif test x"${ac_cv_header_mozldap_ldap_h}" = xyes; then
      AC_EGREP_HEADER(LDAP_REBIND_PROC, mozldap/ldap.h,
         AC_DEFINE(HAVE_LDAP_REBIND_PROC, 1, [Define to 1 if you have LDAP_REBIND_PROC]))
   else
      AC_MSG_CHECKING([for LDAP_REBIND_PROC])
      AC_DEFINE(HAVE_LDAP_REBIND_PROC, 0, [Define to 1 if you have LDAP_REBIND_PROC])
      AC_MSG_RESULT(no)
   fi
dnl
dnl Check for LDAP_REBIND_FUNCTION
dnl
   AC_EGREP_CPP(LDAP_REBIND_FUNCTION, [#define LDAP_REFERRALS
#if HAVE_LDAP_H
#include <ldap.h>
#elif HAVE_MOZLDAP_LDAP_H
#include <mozldap/ldap.h>
#endif
],
      AC_DEFINE(HAVE_LDAP_REBIND_FUNCTION, 1, [Define to 1 if you have LDAP_REBIND_FUNCTION]))

dnl
dnl Check for LDAP_SCOPE_DEFAULT
dnl
   AC_MSG_CHECKING([for LDAP_SCOPE_DEFAULT])
   AC_TRY_RUN([
#if HAVE_LDAP_H
#include <ldap.h>
#elif HAVE_MOZLDAP_LDAP_H
#include <mozldap/ldap.h>
#endif

int
main(void)
{
#ifdef LDAP_SCOPE_DEFAULT
        return 0;
#else
        return 1;
#endif
}], [AC_DEFINE(HAVE_LDAP_SCOPE_DEFAULT, 1, [Define to 1 if you have LDAP_SCOPE_DEFAULT])
   AC_MSG_RESULT(yes)],
   AC_MSG_RESULT(no))

dnl
dnl Check for ldap_url_desc.lud_scheme
dnl
   AC_CHECK_MEMBER(struct ldap_url_desc.lud_scheme,
      AC_DEFINE(HAVE_LDAP_URL_LUD_SCHEME, 1, [Define to 1 if you have LDAPURLDesc.lud_scheme]),,
[#if HAVE_LDAP_H
#include <ldap.h>
#elif HAVE_MOZLDAP_LDAP_H
#include <mozldap/ldap.h>
#endif
])

dnl
dnl Check for ldapssl_client_init
dnl
   if test x"${ac_cv_lib_ldap_main}" = xyes; then
      AC_CHECK_LIB(ldap, ldapssl_client_init,
         AC_DEFINE(HAVE_LDAPSSL_CLIENT_INIT, 1, [Define to 1 if you have ldapssl_client_init]),)
   elif test x"${ac_cv_lib_ssldap60_main}" = xyes; then
      AC_CHECK_LIB(ssldap60, ldapssl_client_init,
         AC_DEFINE(HAVE_LDAPSSL_CLIENT_INIT, 1, [Define to 1 if you have ldapssl_client_init]),)
   else
      AC_MSG_CHECKING([for ldapssl_client_init])
      AC_DEFINE(HAVE_LDAPSSL_CLIENT_INIT, 0, [Define to 1 if you have ldapssl_client_init])
      AC_MSG_RESULT(no)
   fi
dnl
dnl Check for ldap_url_desc2str
dnl
   if test x"${ac_cv_lib_ldap_main}" = xyes; then
      AC_CHECK_LIB(ldap, ldap_url_desc2str,
         AC_DEFINE(HAVE_LDAP_URL_DESC2STR, 1, [Define to 1 if you have ldap_url_desc2str]),)
   elif test x"${ac_cv_lib_ldap60_main}" = xyes; then
      AC_CHECK_LIB(ldap60, ldap_url_desc2str,
         AC_DEFINE(HAVE_LDAP_URL_DESC2STR, 1, [Define to 1 if you have ldap_url_desc2str]),)
   else
      AC_MSG_CHECKING([for ldap_url_desc2str])
      AC_DEFINE(HAVE_LDAP_URL_DESC2STR, 0, [Define to 1 if you have ldap_url_desc2str])
      AC_MSG_RESULT(no)
   fi

dnl
dnl Check for ldap_url_parse
dnl
   if test x"${ac_cv_lib_ldap_main}" = xyes; then
      AC_CHECK_LIB(ldap, ldap_url_parse,
         AC_DEFINE(HAVE_LDAP_URL_PARSE, 1, [Define to 1 if you have ldap_url_parse]),)
   elif test x"${ac_cv_lib_ldap60_main}" = xyes; then
      AC_CHECK_LIB(ldap60, ldap_url_parse,
         AC_DEFINE(HAVE_LDAP_URL_PARSE, 1, [Define to 1 if you have ldap_url_parse]),)
   else
      AC_MSG_CHECKING([for ldap_url_parse])
      AC_DEFINE(HAVE_LDAP_URL_PARSE, 0, [Define to 1 if you have ldap_url_parse])
      AC_MSG_RESULT(no)
   fi
fi
