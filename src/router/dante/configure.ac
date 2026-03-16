AC_INIT
AC_PREREQ(2.61)
AC_REVISION($Id: configure.ac,v 1.661.4.6.2.3.4.13.4.6 2024/12/02 18:45:36 karls Exp $)

unset TAG
lcase () {
      tr 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' 'abcdefghijklmnopqrstuvwxyz'
}
ucase () {
      tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
}

#current software version, typically three sets of numbers (x.y.z).
#can have an added tag, giving a version such as x.y.z-tag.
version=1.4.4
#TAG=""

#NOTE: rpm will not accept '-' chars i version numbers
unset lcTAG
if test x"$TAG" != x; then
   lctag=`echo $TAG | lcase`
fi
RPMVERSION1="$version"
#set this to enable prerelease, changes some defaults (debug enabled)
#prerelease="1"
if test x"$prerelease" != x; then
    prename="pre$prerelease"
    version="${version}${TAG:+-}$TAG-$prename"
    RPMVERSION2="$lctag${lctag:+.}0.$prename"
    TOPEXTRADIST="${TOPEXTRADIST} PRERELEASE"
else
    version="${version}${TAG:+-}$TAG"
    RPMVERSION2="$lctag${lctag:+.}1"
fi

AC_SUBST(RPMVERSION1)
AC_SUBST(RPMVERSION2)

unset initCC initCPP initCFLAGS initCPPFLAGS initLDFLAGS initLIBS
if test x"$CC" != x; then
    initCC="($CC)"
fi
if test x"$CPP" != x; then
    initCPP="($CPP)"
fi
if test x"$CFLAGS" != x; then
    initCFLAGS="($CFLAGS)"
fi
if test x"$CPPFLAGS" != x; then
    initCPPFLAGS="($CPPFLAGS)"
fi
if test x"$LDFLAGS" != x; then
    initLDFLAGS="($LDFLAGS)"
fi
if test x"$LIBS" != x; then
    initLIBS="($LIBS)"
fi

APP=dante
UCAPP=Dante
SERVNAME=sockd
echo "Configuring $UCAPP ${version}:"

case $host in
    *-*-darwin*)
	#XXX problem with -MD and -arch values
	enable_dependency_tracking=no
	AC_MSG_WARN([disabling dependency tracking on this platform])
	;;
esac

AM_INIT_AUTOMAKE(dante, ${version})
AC_CONFIG_SRCDIR(include/common.h)
AM_CONFIG_HEADER(include/autoconf.h)

AC_CONFIG_MACRO_DIR([m4])

#NOTE: save CFLAGS; wish to compile without -O2 when debugging
oCFLAGS="$CFLAGS"
oLDFLAGS="$LDFLAGS"
unset CFLAGS
unset LDFLAGS
LT_INIT
autoconf_compflags="$CFLAGS"
CFLAGS=$oCFLAGS
LDFLAGS="$oLDFLAGS"

unset CONFVAR

#compile silently, if possible
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_DEFINE(BAREFOOTD, 0, [we are Dante])
AC_DEFINE(COVENANT, 0, [we are Dante])
m4_define(dantebuild, yes)

AC_DEFINE_UNQUOTED(PRODUCT, "$UCAPP", [Product name])
AC_DEFINE_UNQUOTED(LCPRODUCT, "$APP", [Product name (lower case)])

AC_ARG_ENABLE(release,
[  --enable-release        build prerelease as full release],
[unset prerelease])

if test x"$prerelease" != x; then
    AC_DEFINE(PRERELEASE, 1, [software prerelease])
else
    AC_DEFINE(PRERELEASE, 0, [software prerelease])
fi
AM_CONDITIONAL(PRERELEASE, test x$prerelease != x)

dnl compiler tests, sets 'warn' with warnings, added to CFLAGS below
m4_include(compiler.m4)

dnl platform functionality tests
m4_include(osdep.m4)

dnl compatibility library tests
unset COMPATFUNCS
m4_include(libscompat.m4)

dnl options for dante
m4_include(danteopt.m4)

dnl common options
unset have_libcfail
m4_include(sharedopt.m4)

dnl generate socket option code
m4_include(sockopt.m4)

#look for modules, and check version
servdir=sockd
unset MOD_BANDWIDTH MOD_REDIRECT MOD_LDAP MOD_PAC MOD_CONVSID
L_MODVER(bandwidth,   109, $servdir, [MOD_BANDWIDTH=t])
L_MODVER(redirect,    79, $servdir, [MOD_REDIRECT=t])
L_MODVER(ldap,        211, $servdir, [MOD_LDAP=t])
L_MODVER(pac,         30, $servdir, [MOD_PAC=t])
L_MODVER(checkmodule, 80, $servdir, [], nokey) #helper file
L_MODVER(convert_sid, 1, "bin", [MOD_CONVSID=t], nokey)

if test x"$MOD_BANDWIDTH" != x; then
   FEAT="$FEAT${FEAT:+ }mod-bandwidth"
fi
unset have_ldap
if test x"$MOD_LDAP" != x -a x"${no_ldap}" = x -a x"${no_gssapi}" = x; then
   FEAT="$FEAT${FEAT:+ }mod-ldap"
   SOCKDDEPS="${SOCKDDEPS}${SOCKDDEPS:+ }$LDAPLIBS"
   AC_DEFINE(HAVE_LDAP, 1, [LDAP support])
   have_ldap=t
fi
unset have_pac
if test x"$MOD_PAC" != x -a x"$MOD_LDAP" -a x"${no_ldap}" = x \
     -a x"${no_pac}" = x -a x"${no_gssapi}" = x -a x"${no_krb5}" = x; then
   FEAT="$FEAT${FEAT:+ }mod-pac"
   AC_DEFINE(HAVE_PAC, 1, [PAC support])
   have_pac=t
   if test x"${have_ldap}" = x; then
      SOCKDDEPS="${SOCKDDEPS}${SOCKDDEPS:+ }$LDAPLIBS"
   fi
fi
AM_CONDITIONAL(HAVEMOD_PAC, test x"${have_pac}" != x)

if test x"$MOD_REDIRECT" != x; then
   FEAT="$FEAT${FEAT:+ }mod-redirect"
fi
FEAT="$FEAT${FEAT:+ }sess2" #session code now part of base

if test x"$prerelease" != x; then
   FEAT="$FEAT${FEAT:+ }prerel" #prerelease build
fi

#standard features
FEAT="$FEAT${FEAT:+ }mon-data mon-disconnect" #monitoring code

#construct SUBDIRS variable
SUBDIRS="unlicensed example doc bin SPECS capi contrib"
DISTSUBDIRS="$SUBDIRS"
UNITSUBDIRS="" #dirs to do unit testing in
if test x"$NOSERVER" = x; then
   SUBDIRS="sockd ${SUBDIRS}"
   UNITSUBDIRS="sockd ${UNITSUBDIRS}"
fi
DISTSUBDIRS="$DISTSUBDIRS sockd"
if test x"$NOCLIENT" = x; then
   if test x"${no_preload_client}" = x; then
       SUBDIRS="dlib $SUBDIRS"
       UNITSUBDIRS="dlib ${UNITSUBDIRS}"
       if test x"$sol64" != x; then
           SUBDIRS="dlib64 $SUBDIRS"
           UNITSUBDIRS="dlib64 ${UNITSUBDIRS}"
       fi
       FEAT="$FEAT${FEAT:+ }preload"
   fi
   #lib handles yacc/lex generation for dlib/sockd, should be before these
   SUBDIRS="lib $SUBDIRS"
   UNITSUBDIRS="lib ${UNITSUBDIRS}"
fi
DISTSUBDIRS="include libscompat $DISTSUBDIRS lib dlib dlib64"

#include should be done before compilation to ensure that
#redefac is executed first
TOPSUBDIRS="$subdirs include"
#libscompat no longer built directly, but build for prereleases for testing
if test x"${build_libscompat}" != x; then
    TOPSUBDIRS="$TOPSUBDIRS libscompat"
    LIBSCOMPAT="-lscompat"
    LDFLAGS="$LDFLAGS${LDFLAGS:+ }-L../libscompat"
else
    unset LIBSCOMPAT
fi
TOPSUBDIRS="$TOPSUBDIRS $SUBDIRS"
AC_SUBST(TOPSUBDIRS)
AC_SUBST(DISTSUBDIRS)
AC_SUBST(TOPEXTRADIST)
AC_SUBST(DLIBDEPS)
AC_SUBST(SOCKDDEPS)
AC_SUBST(LIBSCOMPAT)
AC_SUBST(UNITSUBDIRS)

#set build features for server -v option
FEATURES=`echo $FEAT | xargs -n 1 echo | sort | xargs echo`
AC_DEFINE_UNQUOTED(DANTE_BUILD, "$FEATURES", [Features enabled in build])

#add libcfail
if test x"$have_libcfail" != x; then
    LIBS="$LIBS${LIBS:+ }-lcfail"
fi

#add any warning flags (value set above)
CFLAGS="$CFLAGS $warn"

#store build environment
cp /dev/null buildenv.txt
echo "flags: ${ac_configure_args}" >> buildenv.txt
echo "CC: $CC $initCC" >> buildenv.txt
echo "CFLAGS: $CFLAGS $initCFLAGS" >> buildenv.txt
echo "CPP: $CPP $initCPP" >> buildenv.txt
echo "CPPFLAGS: $CPPFLAGS $initCPPFLAGS" >> buildenv.txt
echo "LDFLAGS: $LDFLAGS $initLDFLAGS" >> buildenv.txt
echo "LD_LIBRARY_PATH: ${LD_LIBRARY_PATH}" >> buildenv.txt
echo "LIBS: $LIBS $initLIBS" >> buildenv.txt
echo "DLIBDEPS: $DLIBDEPS" >> buildenv.txt
echo "SOCKDDEPS: $SOCKDDEPS" >> buildenv.txt
echo "compiler flags: $comp_flags" >> buildenv.txt
echo "warning flags: $warn" >> buildenv.txt
echo "FEATURES: $FEAT" >> buildenv.txt
echo "compat: $COMPATFUNCS" >> buildenv.txt
echo "socket options (socket level): `echo ${UCOKSOCKOPTS_SO} | ucase`" >> buildenv.txt
echo "socket options (ipv4 level): `echo ${UCOKSOCKOPTS_IPV4} | ucase`" >> buildenv.txt
echo "socket options (ipv6 level): `echo ${UCOKSOCKOPTS_IPV6} | ucase`" >> buildenv.txt
echo "socket options (tcp level): `echo ${UCOKSOCKOPTS_TCP} | ucase`" >> buildenv.txt
echo "socket options (udp level): `echo ${UCOKSOCKOPTS_UDP} | ucase`" >> buildenv.txt
echo "socket option arguments: `echo $OKSOCKOPTVALSYMS | ucase`" >> buildenv.txt

cat buildenv.txt

echo "#build environment
export CC=\"$CC\"
export CFLAGS=\"$CFLAGS\"
export CPP=\"$CPP\"
export CPPFLAGS=\"$CPPFLAGS\"
export LDFLAGS=\"$LDFLAGS\"
export SOCKDDEPS=\"$SOCKDDEPS\"
export LIBS=\"$LIBS\"" > buildenvrc

AC_CONFIG_FILES(sockd/Makefile include/Makefile lib/Makefile dlib/Makefile)
AC_CONFIG_FILES(Makefile example/Makefile doc/Makefile bin/Makefile)
AC_CONFIG_FILES(capi/Makefile SPECS/Makefile SPECS/dante.spec)
AC_CONFIG_FILES(libscompat/Makefile contrib/Makefile)
AC_CONFIG_FILES(unlicensed/Makefile VERSION dlib64/Makefile)

#change all #undef's to #define foo 0
AH_BOTTOM([
#include "redefac.h"
])

AC_OUTPUT

rm -f sockd/config_scan.c sockd/config_parse.c
(cd sockd; ln -sf ../lib/config_scan.c config_scan.c)
(cd sockd; ln -sf ../lib/config_parse.c config_parse.c)
(cd dlib; ln -sf ../lib/config_scan.c config_scan.c)
(cd dlib; ln -sf ../lib/config_parse.c config_parse.c)

(cd sockd; ln -sf ../lib/config_parse.y config_parse.y)
(cd sockd; ln -sf ../lib/config_scan.l config_scan.l)
(cd dlib; ln -sf ../lib/config_parse.y config_parse.y)
(cd dlib; ln -sf ../lib/config_scan.l config_scan.l)

echo ""
echo "                     Configure status:"
echo ""

if test x"$NOCLIENT" != x; then
    echo "Client:            $NOCLIENT"
else
    echo "Client:            Enabled"
fi
if test x"$NOSERVER" != x; then
    echo "Server:            $NOSERVER"
else
    echo "Server:            Enabled"
fi
if test x"${no_preload}" = xt; then
   if test x"${blocked_preload}" = xt; then
	echo "Preloading:        Client preloading might not be reliable on this platform"
	CONFVAR="${CONFVAR}${CONFVAR:+ }badpreload"
   else
	echo "Preloading:        Disabled"
	CONFVAR="${CONFVAR}${CONFVAR:+ }nopreload"
   fi
else
    echo "Preloading:        Enabled"
    CONFVAR="${CONFVAR}${CONFVAR:+ }preload"
fi
if test x"${nb_select_err}" = xt; then
    echo "select:            Unexpected select behaviour on unconnected sockets,"
    echo "                   operations on nonblocking sockets might fail "
    echo "                   on this platform when using socksify"
    CONFVAR="${CONFVAR}${CONFVAR:+ }selecterr"
#else
#    echo "select:            OK"
fi
if test x"${no_routeinfo}" = xt; then
    echo "Routeinfo:         Not supported on this platform"
    CONFVAR="${CONFVAR}${CONFVAR:+ }norouteinfo"
else
#    echo "Routeinfo:         OK"
    CONFVAR="${CONFVAR}${CONFVAR:+ }routeinfo"
fi
if test x"$NOLIBWRAP" != x; then
	echo "Libwrap:           $NOLIBWRAP"
else
	echo "Libwrap:           Enabled"
fi
if test x"$NOBSDAUTH" != x; then
	echo "BSD Auth:          $NOBSDAUTH"
else
	echo "BSD Auth:          Enabled"
fi
if test x"$NOPAM" != x; then
	echo "PAM:               $NOPAM"
else
	echo "PAM:               Enabled"
fi
if test x"${no_gssapi}" = xt; then
   if test x"${bad_gssapi}" = xt; then
	echo "GSSAPI:            Located gssapi library not usable"
   else
	echo "GSSAPI:            Not found/disabled"
   fi
else
	echo "GSSAPI:            Enabled"
fi
if test x"${no_krb5}" = xt; then
	echo "KRB5:              Not found/disabled"
else
	echo "KRB5:              Enabled"
fi
if test x"${no_sasl}" = xt; then
	echo "SASL:              Not found/disabled"
else
	echo "SASL:              Enabled"
fi
if test x"${no_upnp}" = xt; then
    if test x"${have_libminiupnp}" = xt; then
	echo "UPNP:              Unsupported miniupnp library version"
    else
	echo "UPNP:              Not found/disabled"
    fi
else
    echo "UPNP:              Enabled"
fi
if test x"$COMPATFUNCS" = x; then
    echo "Compatability:     none"
else
    echo "Compatability:     $COMPATFUNCS"
fi

echo ""
echo "                     Modules:"
echo ""
if test x"$MOD_REDIRECT" != x; then
    echo "redirect:          Enabled"
else
    echo "redirect:          Not found"
fi
if test x"$MOD_BANDWIDTH" != x; then
    echo "bandwidth:         Enabled"
else
    echo "bandwidth:         Not found"
fi
if test x"$MOD_LDAP" != x; then
    if test x"${no_ldap}" != x; then
	s="             " #indent
	echo "ldap:$s Disabled ($noldap)"
	echo "     $s (NOTE: Both libraries and header files for"
	echo "     $s  Kerberos/SASL/GSSAPI/LDAP must be installed for the"
	echo "     $s  LDAP module to compile. On some platforms header files"
	echo "     $s  are found in separate packages (such as gssapi-devel).)"
    else
	echo "ldap:              Enabled"
    fi
else
    echo "ldap:              Not found"
fi
if test x"$MOD_PAC" != x; then
    if test x"${have_pac}" != x; then
        echo "pac:               Enabled"
    else
        s="             " #indent
        echo "pac: $s Disabled ($nopac)"
	echo "     $s (NOTE: Both libraries and header files for"
	echo "     $s Kerberos/SASL/GSSAPI/LDAP must be installed for the"
	echo "     $s PAC module to compile. On some platforms header files"
	echo "     $s are found in separate packages (such as gssapi-devel).)"
    fi
else
    echo "pac:               Not found"
fi

echo ""

echo "CONFRES: $CONFVAR" >> buildenv.txt

if test x"$prerelease" != x; then
   echo "
This is a pre-release.  We ask that everyone who can, test it to the
extent possible, to help reduce problems in the upcoming release.
Problems can be reported to dante-bugs@inet.no.

Note that pre-releases are often configured in a way that can
significantly increase the load on the machine.  This is done to
stress the server more, increasing the chances of detecting potential
problems.
"
fi

VINFO=README.latest
test -s "$VINFO" && cat "$VINFO"

#XXX depcomp regen hack
touch _reconf
for dir in bin lib dlib sockd unlicensed; do
    touch $dir/_reconf
done

true #ensure $? is zero

exit 0
