case $host in
    *-*-osf*)
	AC_MSG_WARN([OSF support might be removed in the near future.])
	AC_MSG_WARN([Please contact dante-bugs@inet.no if you are using this platform.])
	exit 1
	;;
esac

#known keywords for --enable/disable-foo(=yes/no)?
LTINTERNAL="dlopen|dlopen_self|dlopen_self_static|fast_install|libtool_lock|win32_dll|shared_with_static_runtimes|shared_with_static_runtimes_CXX|shared_with_static_runtimes_F77|option_checking|silent_rules"
KNOWN_KEYWORDS="$LTINTERNAL|shared|static|debug|warnings|diagnostic|fsanitize|profiling|coverage|linting|libwrap|preload|serverdl|clientdl|internal|pidfile|drt_fallback|release|dependency_tracking|largefile|livedebug|clientbuild|serverbuild|client|server|libcfail|libcassert|maintainer_mode"
for keyword in `set | grep -E '^enable_' | sed -e 's/^enable_//' | \
                sed -e 's/=.*$//'`; do
    echo $keyword | grep -E "^(${KNOWN_KEYWORDS})$" > /dev/null
    if test $? -ne 0; then
	AC_MSG_WARN([unknown --enable/disable keyword '$keyword'])
	#check is not entirely reliable, only exit in prerelease
	if test x"$prerelease" != x; then
	    AC_MSG_FAILURE([unknown configure keyword, exiting])
	fi
    fi
done

#known keywords for --with/without-foo?
LTINTERNAL="gnu_ld|pic|tags|gnu_ldcxx|sysroot|aix_soname"
KNOWN_KEYWORDS="$LTINTERNAL|socks_conf|sockd_conf|pidfile|iomax|negmax|bufsize|libc|upnp|pam|bsdauth|full_env|gssapi_path|gssapi|krb5_config|krb5|krb5_path|pac|ldap|ldap_path|sasl|sasl_path|glibc_secure|libwrap"
for keyword in `set | grep -E '^with_' | sed -e 's/^with_//' | \
                sed -e 's/=.*$//'`; do
    echo $keyword | grep -E "^(${KNOWN_KEYWORDS})$" > /dev/null
    if test $? -ne 0; then
	AC_MSG_WARN([unknown --with/without keyword '$keyword'])
	#check is not entirely reliable, only exit in prerelease
	if test x"$prerelease" != x; then
	    AC_MSG_FAILURE([unknown configure keyword, exiting])
	fi
    fi
done

AH_TOP([/* avoid warnings on Android */
#ifdef HAVE_ANDROID_OS
# undef /*XXX protect against redefinition */ HAVE_SCHED_SETSCHEDULER
# undef /*XXX protect against redefinition */ HAVE_MALLOC_H
#endif /* HAVE_ANDROID_OS */
])

unset NOCLIENT
AC_MSG_CHECKING([whether client compilation should be disabled])
AC_ARG_ENABLE(client,
[  --disable-client        disable compilation of client library],
[if test x"$enableval" = xno; then
    NOCLIENT="Disabled, using --disable-client"
 fi])
if test x"$NOCLIENT" != x; then
    AC_MSG_RESULT([no])
    CONFVAR="${CONFVAR}${CONFVAR:+ }noclient"
else
    AC_MSG_RESULT([yes])
    CONFVAR="${CONFVAR}${CONFVAR:+ }client"
fi

unset NOSERVER
AC_MSG_CHECKING([whether server compilation should be disabled])
AC_ARG_ENABLE(server,
[  --disable-server        disable compilation of server],
[if test x"$enableval" = xno; then
    NOSERVER="Disabled, using --disable-server"
 fi])
if test x"$NOSERVER" != x; then
    AC_MSG_RESULT([no])
    CONFVAR="${CONFVAR}${CONFVAR:+ }noserver"
else
    AC_MSG_RESULT([yes])
    CONFVAR="${CONFVAR}${CONFVAR:+ }server"
fi

if test x"$NOCLIENT" != x -a x"$NOSERVER" != x; then
    AC_MSG_WARN(cannot disable both client and server compilation)
    exit 1
fi

#allow default file locations to be overridden
unset SOCKSCONFPATH
AC_MSG_CHECKING([for client configuration file location])
AC_ARG_WITH(socks-conf,
[  --with-socks-conf=FILE  change location of socks client configuration file],
[SOCKSCONFPATH="$withval"],
[#set default socks.conf path
 SOCKSCONFPATH="/etc/socks.conf"])
AC_DEFINE_UNQUOTED(SOCKS_CONFIGFILE, "$SOCKSCONFPATH", [socks config file])
AC_MSG_RESULT([$SOCKSCONFPATH])

unset SOCKDCONFPATH
AC_MSG_CHECKING([for server configuration file location])
AC_ARG_WITH(sockd-conf,
[  --with-sockd-conf=FILE  change location of socks server configuration file],
[SOCKDCONFPATH="$withval"],
[#set default sockd.conf path
 SOCKDCONFPATH="/etc/${SERVNAME}.conf"])
AC_DEFINE_UNQUOTED(SOCKD_CONFIGFILE, "$SOCKDCONFPATH", [sockd config file])
AC_MSG_RESULT([$SOCKDCONFPATH])

unset NOBSDAUTH
AC_CHECK_HEADERS(bsd_auth.h)
AC_CHECK_FUNC(auth_userokay)
AC_MSG_CHECKING([for bsd authentication])
AC_ARG_WITH(bsdauth,
[  --without-bsdauth       disable bsdauth support @<:@default=detect@:>@],
[if test x"$withval" = xno; then
    NOBSDAUTH="Disabled, using --without-bsdauth"
 fi])

if test x"$NOBSDAUTH" != x; then
    AC_MSG_RESULT([disabled])
else
    #look for bsd authentication support
    if test x"${ac_cv_header_bsd_auth_h}" = xno; then
	NOBSDAUTH="Disabled, usable bsd_auth.h not found"
	AC_MSG_RESULT([no, usable bsd_auth.h not found])
    else
	if test x"${ac_cv_func_auth_userokay}" = xno; then
	    NOBSDAUTH="Disabled, auth_userokay function not found"
	    AC_MSG_RESULT([no, auth_userokay function not found])
	else
	    AC_DEFINE(HAVE_BSDAUTH, 1, [BSD Authentication support])
	    FEAT="$FEAT${FEAT:+ }bsdauth"
	    AC_MSG_RESULT([yes])
	fi
    fi
fi

#only relevant for platforms lacking issetugid()
AC_MSG_CHECKING([full environment usage])
AC_ARG_WITH(full-env,
[  --without-full-env      restrictive environment variable usage @<:@default=with@:>@],
[CONFENV=$withval])
if test x"$CONFENV" = xno; then
    AC_MSG_RESULT([no])
    AC_DEFINE(HAVE_CONFENV_DISABLE, 1, [Disable environment variables])
else
    AC_MSG_RESULT([yes])
fi

AC_MSG_CHECKING([direct route fallback in client enabled])
AC_ARG_ENABLE(drt-fallback,
[  --enable-drt-fallback   enable direct route fallback in client @<:@default=disabled@:>@],
[],
[#disable fallback by default
 enable_drt_fallback=no])
if test x"${enable_drt_fallback}" = xyes; then
    AC_DEFINE(SOCKS_DIRECTROUTE_FALLBACK, 1, [use fallback])
    AC_MSG_RESULT([yes])
else
    AC_DEFINE(SOCKS_DIRECTROUTE_FALLBACK, 0, [do not use fallback])
    AC_MSG_RESULT([no])
fi

m4_include(preload.m4)

dnl determine GSSAPI/KERBEROS/LDAP/SASL support
no_gssapi=t
no_krb5=t
no_ldap=t
no_sasl=t
no_pac=t

unset noldap nopac
unset LDAPLIBS
m4_include(gssapi.m4)
if test x"${no_gssapi}" = xt; then
   noldap="working GSSAPI installation required"
   nopac="working GSSAPI installation required"
else
   FEAT="$FEAT${FEAT:+ }gssapi"

   LIBScpy="$LIBS"
   m4_include(kerberos.m4)
   if test x"${no_krb5}" = xt; then
       noldap="KerberosV installation required"
       nopac="KerberosV installation required"
   else
       if test x"${no_pac}" = xt; then
          nopac="PAC support needed"
       fi

       m4_include(sasl.m4)
       if test x"${no_sasl}" = xt; then
           noldap="SASL installation required"
       else
           m4_include(ldap.m4)
           if test x"${no_ldap}" = xt; then
               noldap="LDAP installation required"
           else
               if test x"$LIBS" != x; then
                   LDAPLIBS=${LIBS##$LIBScpy} #ensure only new libraries are added
               fi
           fi
       fi
   fi
   if test x"$LIBScpy" != x; then
      LIBS="$LIBScpy"
   fi
fi
if test x"$noldap" != x; then
   AC_MSG_WARN([ldap disabled: $noldap])
fi

dnl workaround for newer glibc versions (and opensolaris)
unset stdio_preload
case $host in
    *-*-linux* | *-*-solaris*)
	#only do preloading if gssapi is defined
	if test x"${no_gssapi}" != xt; then
	    stdio_preload=t
	fi
	;;
esac

dnl 'generate' capi/socks.h file
AC_CONFIG_FILES(capi/socks.h)
if test x"${stdio_preload}" = xt; then
    AC_DEFINE(HAVE_LINUX_GLIBC_WORKAROUND, 1, [stdio function preloading])
    #append contents for stdio preloading
    cat capi/socks_glibc.h >> capi/socks.h
fi

dnl preparation for library mapfile usage to control symbol export
unset MAPOPT
m4_include(mapfile.m4)

#miniupnpc tests
m4_include(miniupnpc.m4)
if test x"${no_upnp}" = x; then
    FEAT="$FEAT${FEAT:+ }upnp"
fi
