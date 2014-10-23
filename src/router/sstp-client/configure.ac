AC_PREREQ([2.65])
AC_INIT([sstp-client], 
        [1.0.9],
        [http://sourceforge.net/projects/sstp-client])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
AC_LANG(C)
AC_CONFIG_SRCDIR([src/sstp-client.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL
LIBEVENT2_MINIMUM=2.0.10

# Check if user asked us to compile with 1.4 support
AC_ARG_WITH(libevent,
    AC_HELP_STRING([--with-libevent], [Specify the libevent version to compile with]),
    [with_libevent="$withval"], [with_libevent="1"])

AS_CASE([$with_libevent],
    ["1"],[AX_CHECK_LIBRARY([LIBEVENT], [event.h], [event], 
            [AC_CHECK_LIB([event], [event_init], [],
                [AC_MSG_ERROR([libevent is not usable])])],
            [AC_MSG_ERROR([Required library libevent not found])])],
    ["2"],[PKG_CHECK_MODULES([LIBEVENT], [libevent >= $LIBEVENT2_MINIMUM],
            [AC_DEFINE([HAVE_LIBEVENT2], [1], [Specify use of libevent >= $LIBEVENT2_MINIMUM])],
            [AC_MSG_ERROR([Required library libevent not found])])],
    ["*"],[AC_MSG_ERROR([Unknown version of libevent specified])])

CFLAGS="$CFLAGS $LIBEVENT_CFLAGS"
LDFLAGS="$LDFLAGS $LIBEVENT_LIBS"

# Check for OpenSSL
AX_CHECK_OPENSSL([], 
    [AC_MSG_ERROR([OpenSSL not found Hint: apt-get install libssl-dev])])
LIBS="$LIBS $OPENSSL_LIBS"
CFLAGS="$CFLAGS $OPENSSL_INCLUDES"
LDFLAGS="$LDFLAGS $OPENSSL_LDFLAGS"


# Check for openpty
AC_CHECK_LIB([util], [openpty])


# Specify privilege separation user
AC_ARG_ENABLE(user,
    AC_HELP_STRING([--enable-user=user], [Drop privileges after start to this user (default: sstpc)]))
AS_IF([ test "${enable_user}" = "yes" || test x"${enable_user}" = x"" ],
    [enable_user="sstpc"],
    [enable_user="root"])
AC_SUBST([enable_user])
AC_DEFINE_UNQUOTED(SSTP_USER, "${enable_user}", The sstpc privilege drop user)


# Specify privilege separation group
AC_ARG_ENABLE(group,
    AC_HELP_STRING([--enable-group=group], [Drop privileges after start to this group (default: sstpc)]))
AS_IF([ test "${enable_group}" = "yes" || test x"${enable_group}" = x"" ],
    [enable_group="sstpc"],
    [enable_group="root"])
AC_SUBST([enable_group])
AC_DEFINE_UNQUOTED(SSTP_GROUP, "${enable_group}", The sstpc privilege drop group)


# Specify runtime directory
AC_ARG_WITH([runtime-dir], 
	AC_HELP_STRING([--with-runtime-dir=DIR], [Specify the runtime directory for sstpc]))
AS_IF([ test -n "$with_runtime_dir"],
	[SSTP_RUNTIME_DIR="$with_runtime_dir"],
	[SSTP_RUNTIME_DIR="${localstatedir}/run/sstpc"])
AC_SUBST(SSTP_RUNTIME_DIR)

# Check to see if we enabled PPP plug-in support (default:yes)
AC_ARG_ENABLE(ppp-plugin, 
    AC_HELP_STRING([--disable-ppp-plugin=DIR], [disable PPP Plugin support]),
    [enable_ppp_plugin=${enableval}], [enable_ppp_plugin=yes])
AS_IF([test "x$enable_ppp_plugin" != "xno"],
    AC_CHECK_HEADER(pppd/pppd.h,,
            AC_MSG_ERROR([pppd.h missing Hint: apt-get install ppp-dev]))
    AC_DEFINE(HAVE_PPP_PLUGIN, 1, [Define if you have PPP support]))


# Check to see if the plugin directory was set
AM_CONDITIONAL(WITH_PPP_PLUGIN, test "${enable_ppp_plugin}" = "yes")
AC_ARG_WITH([pppd-plugin-dir], 
    AS_HELP_STRING([--with-pppd-plugin-dir=DIR], [path to the pppd plugins directory]))
if test -n "$with_pppd_plugin_dir" ; then
    PPPD_PLUGIN_DIR="$with_pppd_plugin_dir"
else
    PPPD_PLUGIN_DIR="${libdir}/pppd/2.4.5"
fi
AC_SUBST(PPPD_PLUGIN_DIR)

# Check if we have netlink support
AC_CHECK_HEADER([linux/rtnetlink.h],
    AC_DEFINE(HAVE_NETLINK, 1, [Use netlink to add/remove route]),
    AC_MSG_WARN([Compiling without netlink support]),
    [#include <sys/socket.h>
     #include <linux/netlink.h>])

# Checks for header files.
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([  \
    arpa/inet.h     \
    fcntl.h         \
    netdb.h         \
    paths.h         \
    stdint.h        \
    stdlib.h        \
    string.h        \
    syslog.h        \
    pty.h           \
    sys/types.h     \
    sys/socket.h    \
    unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_CHOWN
AC_FUNC_MALLOC
AC_CHECK_FUNCS([    \
    dup2            \
    gethostname     \
    localtime_r     \
    memmove         \
    memset          \
    mkdir           \
    socket          \
    strcasecmp      \
    strncasecmp     \
    strchr          \
    strdup          \
    strrchr         \
    strstr          \
    strtoul         \
    strtoull])

AC_CONFIG_FILES([Makefile
                 sstp-client-1.0.pc
                 src/Makefile
                 include/Makefile
                 src/libsstp-log/Makefile
                 src/libsstp-api/Makefile
                 src/libsstp-compat/Makefile
                 src/pppd-plugin/Makefile])
AC_OUTPUT

echo "
$PACKAGE_NAME version $PACKAGE_VERSION
   Prefix.........: $prefix
   Runtime Dir....: $SSTP_RUNTIME_DIR
   PPP Plugin Dir.: $PPPD_PLUGIN_DIR
   User:..........: $enable_user
   Group:.........: $enable_group
   Using OpenSSL..: $OPENSSL_INCLUDES $OPENSSL_LDFLAGS $OPENSSL_LIBS
   C Compiler.....: $CC $CFLAGS
   Using Event....: $LIBEVENT_CFLAGS $LIBEVENT_LIBS
   Linker.........: $LD $LDFLAGS $LIBS
   "

