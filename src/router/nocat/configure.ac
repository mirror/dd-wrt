# Process this file with autoconf to produce a configure script.
AC_INIT(nocat, [0.93pre2], [schuyler@nocat.net], [NoCatSplash])
AC_CONFIG_SRCDIR([src/http.c])
AM_CONFIG_HEADER(src/config.h)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lsplash':
# AC_CHECK_LIB([splash], [http_request_new])
AM_PATH_GLIB( 1.2.0, :, AC_MSG_ERROR([You must have glib-1.2 installed.]), : )

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MMAP
AC_TYPE_SIGNAL
AC_FUNC_STRTOD
AC_CHECK_FUNCS([munmap socket strchr strcspn strrchr strstr strtol])

# Make sure we have values for prefix and exec_prefix
if test "$prefix" = "NONE"; then
    prefix="$ac_default_prefix"
fi
if test "$exec_prefix" = "NONE"; then
    exec_prefix="$prefix"
fi
if test "$pkgdatadir" = ""; then
    pkgdatadir="$datadir/$PACKAGE_NAME"
fi

# which g/w mode?
AC_ARG_WITH(mode,
    [  --with-mode             Authentication mode (open) ],
    [case "${withval}" in
       open) splashd_mode="open.c" ;;
       passive) 
	    splashd_mode="passive.c base64.c"
	    AC_CHECK_LIB([crypt], [crypt])
	    ;;
       *) AC_MSG_ERROR(bad value ${withval} for --with-mode) ;;
     esac
    AC_DEFINE_UNQUOTED(NC_SPLASHD_MODE, "${withval}",
	[Which mode the gateway runs in.])
    ],
    splashd_mode="open.c"
)
AC_SUBST(SPLASHD_MODE, "$splashd_mode")

AC_ARG_WITH(remote-splash,
    [  --with-remote-splash   Fetch splash page from elsewhere with libghttp. ],
    AC_CHECK_LIB([ghttp], [ghttp_request_new])
)

# with ghttpd?
AC_ARG_WITH(ghttpd,
    [  --with-ghttpd           Build and install ghttpd, a lightweight httpd.],
    AC_SUBST(GHTTPD_PROGRAM, "ghttpd"),
    :
)

# Check for firewall software
supported_fw=`find libexec/ -type d ! -path "*CVS*" -exec echo -n "{} " \;`
AC_ARG_WITH(firewall,
    [  --with-firewall=FW      Firewall to configure NoCat for.],
    firewall="$withval",
    AC_CHECK_PROGS(firewall, `cd ./libexec; ls | grep -v CVS`, "")
)
if test "$firewall" = ""; then
    AC_MSG_ERROR(No supported firewalls found in \$PATH.)
fi
AC_SUBST(libexecscripts,
    `find libexec/$firewall -type f ! -path "*CVS*" -exec echo -n "{} " \;`)
AC_SUBST(pkglibexecdir, $libexecdir/$PACKAGE_NAME)
eval "NC_FIREWALL_PATH=`eval echo ${pkglibexecdir}`"
AC_DEFINE_UNQUOTED(NC_FIREWALL_PATH, "$NC_FIREWALL_PATH",
    [Where to find firewall scripts?])

# Check for htdocs files
AC_ARG_WITH(docroot,
    [  --with-docroot=PATH     Directory to store HTML templates in.],
    htdocsdir="$withval", htdocsdir="${pkgdatadir}/htdocs")
AC_SUBST(htdocsdir)
AC_SUBST(imagesdir, $htdocsdir/images)
eval "NC_DOCUMENT_ROOT=`eval echo ${htdocsdir}`"
AC_DEFINE_UNQUOTED(NC_DOCUMENT_ROOT, "$NC_DOCUMENT_ROOT",
    [Where to serve documents from?])

# Where do I find nocat.conf?
eval "NC_CONF_PATH=`eval echo ${sysconfdir}`"
AC_DEFINE_UNQUOTED(NC_CONF_PATH, "$NC_CONF_PATH",
    [Where to find our nocat.conf, by default])

eval "NC_STATE_DIR=`eval echo ${localstatedir}`"
AC_DEFINE_UNQUOTED(NC_STATE_DIR, "$NC_STATE_DIR",
    [Where to chdir after daemonizing, by default])

AC_DEFINE_UNQUOTED(NC_PID_FILE, "$NC_STATE_DIR/run/splashd.pid",
    [Where to store our PID file, by default])

# PGP files?
AC_SUBST(pgpdir, "$pkgdatadir/pgp")
eval "NC_PGP_PATH=`eval echo ${pgpdir}`"
AC_DEFINE_UNQUOTED(NC_PGP_PATH, "$NC_PGP_PATH",
    [Where to find our PGP keys?])

# Build makefiles
AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
