AC_PREREQ([2.65])
AC_INIT([dnscrypt-proxy],[1.9.5],[https://dnscrypt.org])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/proxy/app.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_SUBDIRS([src/libevent-modified])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([1.11 dist-bzip2 tar-ustar gnu subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE
AM_DEP_TRACK

AC_SUBST(VERSION)
ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_LANG_ASSERT(C)
LX_CFLAGS=${CFLAGS-NONE}

dnl Switches.

AC_ARG_ENABLE(ssp,
[AS_HELP_STRING(--disable-ssp,Do not compile with -fstack-protector)],
[
  AS_IF([test "x$enableval" = "xno"], [
    enable_ssp="no"
  ], [
    enable_ssp="yes"
  ])
],
[
  enable_ssp="yes"
])

AC_ARG_ENABLE(pie,
[AS_HELP_STRING(--disable-pie,Do not produce position independent executables)],
 enable_pie=$enableval, enable_pie="maybe")

AS_CASE([$host_os], [mingw*], [enable_pie="no"])

AC_ARG_WITH(safecode,
[AS_HELP_STRING(--with-safecode,For maintainers only - please do not use)],
[AS_IF([test "x$withval" = "xyes"], [
    AC_ARG_VAR([SAFECODE_HOME], [set to the safecode base directory])
    : ${SAFECODE_HOME:=/opt/safecode}
    LDFLAGS="$LDFLAGS -L${SAFECODE_HOME}/lib"
    LIBS="$LIBS -lsc_dbg_rt -lpoolalloc_bitmap -lstdc++"
    CFLAGS="$CFLAGS -fmemsafety"
  ])
])

AC_ARG_ENABLE(plugins,
[AS_HELP_STRING(--disable-plugins,Disable support for plugins)],
[
  AS_IF([test "x$enableval" = "xno"], [
    plugins="disabled"
  ], [
    plugins="enabled"
  ])
],
[
  plugins="enabled"
])
AS_IF([test "$plugins" = "enabled"], [
  CPPFLAGS="$CPPFLAGS -DPLUGINS=1"
])
AM_CONDITIONAL(PLUGINS, test x$plugins = xenabled)

AC_ARG_ENABLE(debug,
[AS_HELP_STRING(--enable-debug,For maintainers only - please do not use)],
[
  AS_IF([test "x$enableval" = "xyes"], [
    AS_IF([test "x$LX_CFLAGS" = "xNONE"], [
      nxflags=""
      for flag in `echo $CFLAGS`; do
        AS_CASE([$flag],
          [-O*], [ ],
          [-g*], [ ],
          [*], [AS_VAR_APPEND([nxflags], [" $flag"])])
      done
      CFLAGS="$nxflags -O0 -g3"
    ])
    CPPFLAGS="$CPPFLAGS -DDEBUG=1"
  ])
])

relaxed_plugins_permissions=disabled
AC_ARG_ENABLE(relaxed-plugins-permissions,
[AS_HELP_STRING(--enable-relaxed-plugins-permissions,Allow loading plugins owned by other users)],
[
	AS_IF([test "x$enableval" != "xno"], [
		CPPFLAGS="$CPPFLAGS -DRELAXED_PLUGINS_PERMISSIONS=1"
		relaxed_plugins_permissions=enabled
	])
])
AM_CONDITIONAL(RELAXED_PLUGINS_PERMISSIONS, test x$relaxed_plugins_permissions = xenabled)

plugins_root=disabled
AC_ARG_ENABLE(plugins-root,
[AS_HELP_STRING(--enable-plugins-root,Only load plugins sitting in the default plugins directory)],
[
	AS_IF([test "x$enableval" != "xno"], [
		CPPFLAGS="$CPPFLAGS -DENABLE_PLUGINS_ROOT=1"
		plugins_root=enabled
	])
])
AM_CONDITIONAL(ENABLE_PLUGINS_ROOT, test x$plugins_root = xenabled)

AC_ARG_WITH([systemd],
  AS_HELP_STRING([--with-systemd], [build with support for systemd])
)

have_systemd=no
AS_IF([test "x$with_systemd" = "xyes"], [
  PKG_CHECK_MODULES([SYSTEMD], [libsystemd], [have_systemd=yes], [
    PKG_CHECK_MODULES([SYSTEMD_DAEMON], [libsystemd-daemon], [have_systemd=yes], [have_systemd=no])
  ])
  AS_CASE([$with_systemd:$have_systemd],
    [yes:no],
      [AC_MSG_ERROR([systemd expected but libsystemd not found -- maybe you need to install the pkg-config and libsystemd-dev packages])],
    [*:yes],
       AC_DEFINE([HAVE_LIBSYSTEMD], [1], [Define if libsystemd is available])
  )
])
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$have_systemd" = "xyes"])

AC_SUBST([MAINT])

dnl Checks

AC_PROG_CC_C99
AC_USE_SYSTEM_EXTENSIONS

AX_CHECK_COMPILE_FLAG([-D_FORTIFY_SOURCE=2],
  [CPPFLAGS="$CPPFLAGS -D_FORTIFY_SOURCE=2"])

AS_CASE([$host_os], [cygwin*|mingw*|msys|pw32*|cegcc*], [ ], [
  AX_CHECK_COMPILE_FLAG([-fPIC], [CFLAGS="$CFLAGS -fPIC"])
])

AS_IF([test "$enable_pie" != "no"],[
  AX_CHECK_COMPILE_FLAG([-fPIE], [
    AX_CHECK_LINK_FLAG([-pie], [
      [CFLAGS="$CFLAGS -fPIE"
       LDFLAGS="$LDFLAGS -pie"]
    ])
  ])
])

AX_CHECK_COMPILE_FLAG([-fno-strict-aliasing], [CFLAGS="$CFLAGS -fno-strict-aliasing"])
AX_CHECK_COMPILE_FLAG([-fno-strict-overflow], [CFLAGS="$CFLAGS -fno-strict-overflow"], [
  AX_CHECK_COMPILE_FLAG([-fwrapv], [CFLAGS="$CFLAGS -fwrapv"])
])

AS_CASE([$host_os],
  [cygwin* | mingw* | pw32* | cegcc*], [
    AX_CHECK_LINK_FLAG([-Wl,--dynamicbase], [LDFLAGS="$LDFLAGS -Wl,--dynamicbase"])
    AX_CHECK_LINK_FLAG([-Wl,--high-entropy-va], [LDFLAGS="$LDFLAGS -Wl,--high-entropy-va"])
    AX_CHECK_LINK_FLAG([-Wl,--nxcompat], [LDFLAGS="$LDFLAGS -Wl,--nxcompat"])
  ])

AS_IF([test "x$enable_ssp" != "xno"],[

AS_CASE([$host_os],
  [cygwin* | mingw* | pw32* | cegcc*], [ ],
  [*], [
    AX_CHECK_COMPILE_FLAG([-fstack-protector-strong],
     [CFLAGS="$CFLAGS -fstack-protector-strong"],
     [AX_CHECK_COMPILE_FLAG([-fstack-protector-all],
       [CFLAGS="$CFLAGS -fstack-protector-all"])]
    )
  ])
])

AX_CHECK_COMPILE_FLAG([-Winit-self], [CFLAGS="$CFLAGS -Winit-self"])
AX_CHECK_COMPILE_FLAG([-Wwrite-strings], [CFLAGS="$CFLAGS -Wwrite-strings"])
AX_CHECK_COMPILE_FLAG([-Wdiv-by-zero], [CFLAGS="$CFLAGS -Wdiv-by-zero"])
AX_CHECK_COMPILE_FLAG([-Wsometimes-uninitialized], [CFLAGS="$CFLAGS -Wsometimes-uninitialized"])

AC_ARG_VAR([CWFLAGS], [define to compilation flags for generating extra warnings])

AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wall], [CWFLAGS="$CWFLAGS -Wall"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wextra], [CWFLAGS="$CWFLAGS -Wextra"])

AC_MSG_CHECKING(for clang)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
#ifndef __clang__
be sad
#endif
]])],
  [AC_MSG_RESULT(yes)
   AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wno-unknown-warning-option],
  [CWFLAGS="$CWFLAGS -Wno-unknown-warning-option"])],
  [AC_MSG_RESULT(no)
])

AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wbad-function-cast], [CWFLAGS="$CWFLAGS -Wbad-function-cast"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wcast-align], [CWFLAGS="$CWFLAGS -Wcast-align"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wcast-qual], [CWFLAGS="$CWFLAGS -Wcast-qual"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wchar-subscripts], [CWFLAGS="$CWFLAGS -Wchar-subscripts"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wcomment], [CWFLAGS="$CWFLAGS -Wcomment"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wfloat-equal], [CWFLAGS="$CWFLAGS -Wfloat-equal"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wformat=2], [CWFLAGS="$CWFLAGS -Wformat=2"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wimplicit], [CWFLAGS="$CWFLAGS -Wimplicit"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wmissing-declarations], [CWFLAGS="$CWFLAGS -Wmissing-declarations"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wmissing-prototypes], [CWFLAGS="$CWFLAGS -Wmissing-prototypes"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wnormalized=id], [CWFLAGS="$CWFLAGS -Wnormalized=id"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Woverride-init], [CWFLAGS="$CWFLAGS -Woverride-init"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wparentheses], [CWFLAGS="$CWFLAGS -Wparentheses"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wpointer-arith], [CWFLAGS="$CWFLAGS -Wpointer-arith"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wredundant-decls], [CWFLAGS="$CWFLAGS -Wredundant-decls"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wstrict-prototypes], [CWFLAGS="$CWFLAGS -Wstrict-prototypes"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wswitch-enum], [CWFLAGS="$CWFLAGS -Wswitch-enum"])
AX_CHECK_COMPILE_FLAG([$CWFLAGS -Wvariable-decl], [CWFLAGS="$CWFLAGS -Wvariable-decl"])

AX_CHECK_LINK_FLAG([-Wl,-z,relro], [LDFLAGS="$LDFLAGS -Wl,-z,relro"])
AX_CHECK_LINK_FLAG([-Wl,-z,now], [LDFLAGS="$LDFLAGS -Wl,-z,now"])
AX_CHECK_LINK_FLAG([-Wl,-z,noexecstack], [LDFLAGS="$LDFLAGS -Wl,-z,noexecstack"])

LT_INIT
AC_SUBST(LIBTOOL_DEPS)

AC_ARG_VAR([RONN], [path to the ronn text processor])
AC_PATH_PROG([RONN], [ronn], [#])

AC_ARG_VAR([AR], [path to the ar utility])
AC_CHECK_TOOL([AR], [ar], [ar])

case $host_os in
  darwin*)
    case $build_os in
      darwin*)
        AC_ARG_VAR([DTRACE], [path to dtrace if you have it])
        AC_PATH_PROG([DTRACE], [dtrace], [], [$PATH:/usr/local/sbin:/usr/sbin])
      ;;
    esac
  ;;
esac
AS_IF([test -n "$DTRACE"],[
  PROBES_SOURCE="probes_dnscrypt_proxy.h"
],[
  DTRACE="#"
  PROBES_SOURCE="probes_no_dtrace.h"
])

AC_SUBST([PROBES_SOURCE])

AS_IF([test -d /usr/local/include], [
  CPPFLAGS="$CPPFLAGS -I/usr/local/include"
])
AS_IF([test -d /usr/local/lib], [
  LDFLAGS="$LDFLAGS -L/usr/local/lib"
])

AS_IF([test -d /usr/local/opt/libressl/include], [
  CPPFLAGS="$CPPFLAGS -I/usr/local/opt/libressl/include"
], [
  AS_IF([test -d /usr/local/opt/openssl/include], [
    CPPFLAGS="$CPPFLAGS -I/usr/local/opt/openssl/include"
  ])
])
AS_IF([test -d /usr/local/opt/libressl/lib], [
  LDFLAGS="$LDFLAGS -L/usr/local/opt/libressl/lib"
], [
  AS_IF([test -d /usr/local/opt/openssl/lib], [
    LDFLAGS="$LDFLAGS -L/usr/local/opt/openssl/lib"
  ])
])

dnl Checks for header files.

AC_SYS_LARGEFILE
AC_CHECK_HEADERS([sys/cdefs.h sys/feature_tests.h])
AC_CHECK_HEADERS([execinfo.h paths.h pwd.h grp.h uuid/uuid.h])
AC_CHECK_HEADERS([sandbox.h linux/filter.h linux/random.h])
AC_CHECK_HEADERS([ws2tcpip.h])

dnl Checks for typedefs, structures, and compiler characteristics.

AC_C_INLINE
AC_C_BIGENDIAN
AC_C_RESTRICT

AC_CHECK_TYPE([in_port_t], , [AC_DEFINE(in_port_t, uint16_t,
[in_port_t type])],
[
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif
])

AC_MSG_CHECKING([whether sin_len is defined])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif
]], [[
do {
 struct sockaddr_in a;
 (void) a.sin_len;
} while(0)
]])],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_SIN_LEN,[1],[define if you have sin_len])
],[
  AC_MSG_RESULT(no)
])

AC_MSG_CHECKING([whether __ss_family is defined])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif
]], [[
do {
 struct sockaddr_storage a;
 (void) a.__ss_family;
} while(0)
]])],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE___SS_FAMILY,[1],[define if you have __ss_family])
],[
  AC_MSG_RESULT(no)
])

AC_MSG_CHECKING([whether ss_len is defined])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif
]], [[
do {
 struct sockaddr_storage a;
 (void) a.ss_len;
} while(0)
]])],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_SS_LEN,[1],[define if you have ss_len])
],[
  AC_MSG_RESULT(no)
])

AC_MSG_CHECKING([whether __ss_len is defined])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif
]], [[
do {
 struct sockaddr_storage a;
 (void) a.__ss_len;
} while(0)
]])],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE___SS_LEN,[1],[define if you have __ss_len])
],[
  AC_MSG_RESULT(no)
])

dnl Checks for library functions.

AC_SEARCH_LIBS(pow, [m])
AC_SEARCH_LIBS(dlopen, [dl])

AS_IF([test "$host_vendor" = "apple"], [],[
  AC_SEARCH_LIBS(clock_gettime, [rt],
    [AC_DEFINE(HAVE_CLOCK_GETTIME,[1],[define if you have clock_gettime()])])
])

AC_SEARCH_LIBS(backtrace, [execinfo],
  [AC_DEFINE(HAVE_BACKTRACE,[1],[define if you have backtrace()])])

AC_SEARCH_LIBS(gethostbyname, [resolv nsl ws2_32])
AC_SEARCH_LIBS(recvfrom, [socket])
AC_SEARCH_LIBS(kvm_open, [kvm])
AC_SEARCH_LIBS(sodium_hex2bin, [sodium], [ ], AC_ERROR([libsodium >= 1.0.0 not found]))

use_ldns=no
AC_CHECK_HEADER([ldns/ldns.h], [
  AC_CHECK_LIB(ldns, ldns_wire2pkt, [
    AC_DEFINE(USE_LDNS,[1],[define to use the ldns library])
    LDNS_LIBS=-lldns
    use_ldns=yes
  ])
], [ ],
[#include <sys/types.h>
 #include <stdlib.h>
 #include <stdint.h>
 #ifdef HAVE_WS2TCPIP_H
 # include <ws2tcpip.h>
 #endif
])
AM_CONDITIONAL(USE_LDNS, test x$use_ldns = xyes)
AC_SUBST([LDNS_LIBS])

case $host_os in
  cygwin* | pw32* | cegcc*)
    AC_CHECK_LIB([ws2_32], [_head_libws2_32_a])
    AC_CHECK_LIB([psapi], [_head_libpsapi_a])
    AC_CHECK_LIB([iphlpapi], [_head_libiphlpapi_a])
  ;;
  mingw*)
    case $host_cpu in
      amd64|x86_64)
        AC_CHECK_LIB([ws2_32], [_head_lib64_libws2_32_a])
        AC_CHECK_LIB([psapi], [_head_lib64_libpsapi_a])
        AC_CHECK_LIB([iphlpapi], [_head_lib64_libiphlpapi_a])
      ;;
      *)
        AC_CHECK_LIB([ws2_32], [__head_lib32_libws2_32_a])
        AC_CHECK_LIB([psapi], [_head_lib32_libpsapi_a])
        AC_CHECK_LIB([iphlpapi], [_head_lib32_libiphlpapi_a])
      ;;
    esac
  ;;
esac

AC_CHECK_FUNCS([getpwnam sandbox_init setrlimit putc_unlocked gmtime_r initgroups])
AC_CHECK_FUNCS([crypto_box_easy_afternm crypto_core_hchacha20 crypto_box_curve25519xchacha20poly1305_open_easy_afternm])

dnl Libtool.

LT_INIT([dlopen])
LTDL_INIT

dnl Subconfig.

case $host_os in
  cygwin* | mingw* | pw32* | cegcc*) ;;
  *) ac_configure_args="$ac_configure_args --disable-thread-support" ;;
esac

ac_configure_args="$ac_configure_args --disable-dependency-tracking"
ac_configure_args="$ac_configure_args --disable-libevent-install"
ac_configure_args="$ac_configure_args --disable-openssl"
ac_configure_args="$ac_configure_args --disable-shared"
ac_configure_args="$ac_configure_args --enable-function-sections"
ac_configure_args="$ac_configure_args CC='$CC'"
ac_configure_args="$ac_configure_args CFLAGS='$CFLAGS'"
ac_configure_args="$ac_configure_args CPP='$CPP'"
ac_configure_args="$ac_configure_args CPPFLAGS='$CPPFLAGS'"
ac_configure_args="$ac_configure_args LDFLAGS='$LDFLAGS'"
ac_configure_args="$ac_configure_args LIBS='$LIBS'"

dnl Output.

AH_VERBATIM([NDEBUG], [/* Never ever ignore assertions */
#ifdef NDEBUG
#/**/undef/**/ NDEBUG
#endif])

AC_CONFIG_FILES([Makefile
                 contrib/Makefile
                 dist-build/Makefile
                 man/Makefile
                 src/Makefile
                 src/hostip/Makefile
                 src/proxy/Makefile
                 src/ext/Makefile
                 src/include/Makefile
                 src/include/dnscrypt/version.h
                 src/plugins/Makefile
                 src/plugins/example/Makefile
                 src/plugins/example-cache/Makefile
                 src/plugins/example-logging/Makefile
                 src/plugins/example-ldns-aaaa-blocking/Makefile
                 src/plugins/example-ldns-blocking/Makefile
                 src/plugins/example-ldns-forwarding/Makefile
                 src/plugins/vendor-specific/example-ldns-opendns-deviceid/Makefile
                 src/plugins/vendor-specific/example-ldns-opendns-set-client-ip/Makefile
                 test/Makefile])

AC_OUTPUT
