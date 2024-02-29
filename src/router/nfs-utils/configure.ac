dnl Process this file with autoconf to produce a configure script.
dnl
AC_INIT([linux nfs-utils],[2.6.4],[linux-nfs@vger.kernel.org],[nfs-utils])
AC_CANONICAL_BUILD([])
AC_CANONICAL_HOST([])
AC_CONFIG_MACRO_DIR(aclocal)
AM_INIT_AUTOMAKE
AC_PREREQ(2.59)
AC_PREFIX_DEFAULT(/usr)
AM_MAINTAINER_MODE
AC_USE_SYSTEM_EXTENSIONS

dnl *************************************************************
dnl * Define the set of applicable options
dnl *************************************************************
AC_ARG_WITH(release,
	[AS_HELP_STRING([--with-release=XXX],[set release to XXX [1]])],
	RELEASE=$withval,
	RELEASE=1)
	AC_SUBST(RELEASE)
AC_ARG_WITH(statedir,
	[AS_HELP_STRING([--with-statedir=/foo],[use state dir /foo @<:@default=/var/lib/nfs@:>@])],
	statedir=$withval,
	statedir=/var/lib/nfs)
	AC_SUBST(statedir)
AC_ARG_WITH(nfsconfig,
	[AS_HELP_STRING([--with-nfsconfig=/config/file],[use general config file /config/file @<:@default=/etc/nfs.conf@:>@])],
	nfsconfig=$withval,
	nfsconfig=/etc/nfs.conf)
	AC_SUBST(nfsconfig)
AC_ARG_WITH(statdpath,
	[AS_HELP_STRING([--with-statdpath=/foo],[define the statd state dir as /foo instead of the NFS statedir @<:@default=/var/lib/nfs@:>@])],
	statdpath=$withval,
	statdpath=$statedir
	)
	AC_SUBST(statdpath)
AC_ARG_WITH(statduser,
	[AS_HELP_STRING([--with-statduser=rpcuser],[statd to run under @<:@rpcuser or nobody@:>@
	])],
	statduser=$withval,
	if test "x$cross_compiling" = "xno"; then
		if grep -s '^rpcuser:' /etc/passwd > /dev/null; then
			statduser=rpcuser
		else
			statduser=nobody
		fi
	else
		statduser=nobody
	fi)
	AC_SUBST(statduser)
AC_ARG_WITH(start-statd,
	[AS_HELP_STRING([--with-start-statd=scriptname],[When an nfs filesystems is mounted with locking, run this script
	])],
	startstatd=$withval,
	startstatd=/usr/sbin/start-statd
	)
	AC_SUBST(startstatd)
	AC_DEFINE_UNQUOTED(START_STATD, "$startstatd", [Define this to a script which can start statd on mount])
unitdir=/usr/lib/systemd/system
AC_ARG_WITH(systemd,
	[AS_HELP_STRING([--with-systemd@<:@=unit-dir-path@:>@],[install systemd unit files @<:@Default: no, and path defaults to /usr/lib/systemd/system if not given@:>@])],
	if test "$withval" != "no" ; then 
		use_systemd=1
		if test "$withval" != "yes" ; then 
			unitdir=$withval
		fi
	else
		use_systemd=0
	fi
	)
	AM_CONDITIONAL(INSTALL_SYSTEMD, [test "$use_systemd" = 1])
	AC_SUBST(unitdir)

AC_ARG_ENABLE(nfsv4,
	[AS_HELP_STRING([--disable-nfsv4],[disable support for NFSv4 @<:@default=no@:>@])],
	enable_nfsv4=$enableval,
	enable_nfsv4=yes)
	if test "$enable_nfsv4" = yes; then
		IDMAPD=idmapd
	else
		enable_nfsv4=
		IDMAPD=
	fi
	AC_SUBST(IDMAPD)
	AC_SUBST(enable_nfsv4)
	AM_CONDITIONAL(CONFIG_NFSV4, [test "$enable_nfsv4" = "yes"])

AC_ARG_ENABLE(nfsv41,
	[AS_HELP_STRING([--disable-nfsv41],[disable support for NFSv41 @<:@default=no@:>@])],
	enable_nfsv41=$enableval,
	enable_nfsv41=yes)
	if test "$enable_nfsv41" = yes; then
		if test "$enable_nfsv4" != yes; then
			AC_MSG_WARN([NFS v4 is not enabled. Disabling NFS v4.1])
			enable_nfsv41=no
		fi
		BLKMAPD=blkmapd
	else
		enable_nfsv41=
		BLKMAPD=
	fi
	AC_SUBST(enable_nfsv41)
	AM_CONDITIONAL(CONFIG_NFSV41, [test "$enable_nfsv41" = "yes"])

AC_ARG_ENABLE(gss,
	[AS_HELP_STRING([--disable-gss],[disable client support for rpcsec_gss @<:@default=no@:>@])],
	enable_gss=$enableval,
	enable_gss=yes)
	if test "$enable_gss" = yes; then
		GSSD=gssd
	else
		enable_gss=
		GSSD=
	fi
	AC_SUBST(GSSD)
	AC_SUBST(enable_gss)
	AM_CONDITIONAL(CONFIG_GSS, [test "$enable_gss" = "yes"])

AC_ARG_ENABLE(svcgss,
	[AS_HELP_STRING([--enable-svcgss],[enable building svcgssd for rpcsec_gss server support @<:@default=no@:>@])],
	enable_svcgss=$enableval,
	enable_svcgss=no)
	if test "$enable_gss" = yes -a "$enable_svcgss" = yes; then
		SVCGSSD=svcgssd
	else
		enable_svcgss=
		SVCGSSD=
	fi
	AC_SUBST(SVCGSSD)
	AC_SUBST(enable_svcgss)
	AM_CONDITIONAL(CONFIG_SVCGSS, [test "$enable_svcgss" = "yes"])

AC_ARG_ENABLE(kprefix,
	[AS_HELP_STRING([--enable-kprefix],[install progs as rpc.knfsd etc])],
	test "$enableval" = "yes" && kprefix=k,
	kprefix=)
	AC_SUBST(kprefix)
AC_ARG_WITH(rpcgen,
	[AS_HELP_STRING([--with-rpcgen=internal],[use internal rpcgen instead of system one])],
	rpcgen_path=$withval,
	rpcgen_path=yes )
	rpcgen_cflags=-Werror=strict-prototypes
	RPCGEN_PATH=
	if test "$rpcgen_path" = "yes"; then
	    for p in /usr/local/bin/rpcgen /usr/bin/rpcgen /bin/rpcgen
	    do if test -f $p ; then RPCGEN_PATH=$p ; break; fi ; done
		if test -z "$RPCGEN_PATH"; then
			AC_MSG_ERROR([Please install rpcgen or use --with-rpcgen])
		fi
	elif test "$rpcgen_path" != "internal"; then
	    RPCGEN_PATH=$rpcgen_path
	else
	    RPCGEN_PATH=internal
	    rpcgen_cflags=-Wstrict-prototypes
	fi
	AC_SUBST(RPCGEN_PATH)
	AM_CONDITIONAL(CONFIG_RPCGEN, [test "$RPCGEN_PATH" = "internal"])
AC_ARG_ENABLE(uuid,
	[AS_HELP_STRING([--disable-uuid],[Exclude uuid support to avoid buggy libblkid. @<:@default=no@:>@])],
	if test "$enableval" = "yes" ; then choose_blkid=yes; else choose_blkid=no; fi,
	choose_blkid=default)
AC_ARG_ENABLE(mount,
	[AS_HELP_STRING([--disable-mount],[Do not build mount.nfs and do use the util-linux mount(8) functionality. @<:@default=no@:>@])],
	enable_mount=$enableval,
	enable_mount=yes)
	AM_CONDITIONAL(CONFIG_MOUNT, [test "$enable_mount" = "yes"])

if test "$enable_mount" = yes; then
	AC_ARG_ENABLE(libmount-mount,
		[AS_HELP_STRING([--enable-libmount-mount],[Link mount.nfs with libmount @<:@default=no@:>@])],
		enable_libmount=$enableval,
		enable_libmount=no)
else
	enable_libmount=no
fi

AC_ARG_ENABLE(sbin-override,
	[AS_HELP_STRING([--disable-sbin-override],[Don't force nfsdcltrack and mount helpers into /sbin: always honour --sbindir])],
	enable_sbin_override=$enableval,
	enable_sbin_override=yes)
	AM_CONDITIONAL(CONFIG_SBIN_OVERRIDE, [test "$enable_sbin_override" = "yes"])
AC_ARG_ENABLE(junction,
	[AS_HELP_STRING([--enable-junction],[enable support for NFS junctions @<:@default=no@:>@])],
	enable_junction=$enableval,
	enable_junction=no)
	if test "$enable_junction" = yes; then
		AC_DEFINE(HAVE_JUNCTION_SUPPORT, 1,
                          [Define this if you want junction support compiled in])
	else
		enable_junction=
	fi
	AM_CONDITIONAL(CONFIG_JUNCTION, [test "$enable_junction" = "yes" ])

AC_ARG_ENABLE(tirpc,
	[AS_HELP_STRING([--disable-tirpc],[disable use of TI-RPC library @<:@default=no@:>@])],
	enable_tirpc=$enableval,
	enable_tirpc=yes)
AC_ARG_ENABLE(ipv6,
	[AS_HELP_STRING([--disable-ipv6],[disable support for IPv6 @<:@default=no@:>@])],
	enable_ipv6=$enableval,
	enable_ipv6=yes)
	if test "$enable_ipv6" = yes; then
		AC_DEFINE(IPV6_SUPPORTED, 1, [Define this if you want IPv6 support compiled in])
	else
		enable_ipv6=
	fi
	AC_SUBST(enable_ipv6)
	AM_CONDITIONAL(CONFIG_IPV6, [test "$enable_ipv6" = "yes"])

if test "$enable_mount" = yes; then
	AC_ARG_ENABLE(mountconfig,
	[AS_HELP_STRING([--disable-mountconfig],[disable mount to use a configuration file @<:@default=no@:>@])],
	enable_mountconfig=$enableval,
	enable_mountconfig=yes)
	if test "$enable_mountconfig" = no; then
		enable_mountconfig=
	else
		AC_DEFINE(MOUNT_CONFIG, 1, 
			[Define this if you want mount to read a configuration file])
		AC_ARG_WITH(mountfile,
			[AS_HELP_STRING([--with-mountfile=filename],[Using filename as the NFS mount options file [/etc/nfsmounts.conf]
			])],
		mountfile=$withval,
		mountfile=/etc/nfsmount.conf)
		AC_SUBST(mountfile)
		AC_DEFINE_UNQUOTED(MOUNTOPTS_CONFFILE, "$mountfile", 
			[This defines the location of the NFS mount configuration file])
	fi
	AC_SUBST(enable_mountconfig)
	AM_CONDITIONAL(MOUNT_CONFIG, [test "$enable_mountconfig" = "yes"])
else
	AM_CONDITIONAL(MOUNT_CONFIG, [test "$enable_mount" = "yes"])
fi

AC_ARG_ENABLE(nfsdcld,
	[AS_HELP_STRING([--disable-nfsdcld],[disable NFSv4 clientid tracking daemon @<:@default=no@:>@])],
	enable_nfsdcld=$enableval,
	enable_nfsdcld="yes")

AC_ARG_ENABLE(nfsrahead,
	[AS_HELP_STRING([--disable-nfsrahead],[disable nfsrahead command @<:@default=no@:>@])],
	enable_nfsrahead=$enableval,
	enable_nfsrahead="yes")
	AM_CONDITIONAL(CONFIG_NFSRAHEAD, [test "$enable_nfsrahead" = "yes" ])
	if test "$enable_nfsrahead" = yes; then
		dnl Check for -lmount
		PKG_CHECK_MODULES([LIBMOUNT], [mount])
	fi

AC_ARG_ENABLE(nfsdcltrack,
	[AS_HELP_STRING([--disable-nfsdcltrack],[disable NFSv4 clientid tracking programs @<:@default=no@:>@])],
	enable_nfsdcltrack=$enableval,
	enable_nfsdcltrack="yes")

AC_ARG_ENABLE(nfsv4server,
	[AS_HELP_STRING([--enable-nfsv4server],[enable support for NFSv4 only server  @<:@default=no@:>@])],
	enable_nfsv4server=$enableval,
	enable_nfsv4server="no")
	if test "$enable_nfsv4server" = yes; then
		AC_DEFINE(HAVE_NFSV4SERVER_SUPPORT, 1,
                          [Define this if you want NFSv4 server only support compiled in])
	fi
	AM_CONDITIONAL(CONFIG_NFSV4SERVER, [test "$enable_nfsv4server" = "yes" ])

dnl Check for TI-RPC library and headers
AC_LIBTIRPC

dnl Check for -lcap
AC_LIBCAP

dnl Check for -lxml2
AC_LIBXML2

# Check whether user wants TCP wrappers support
AC_TCP_WRAPPERS

# Arrange for large-file support
AC_SYS_LARGEFILE

dnl Check for getrandom() libc support
AC_GETRANDOM

AC_CONFIG_SRCDIR([support/include/config.h.in])
AC_CONFIG_HEADERS([support/include/config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
LT_INIT
AM_PROG_CC_C_O

if test "x$cross_compiling" = "xno"; then
	CC_FOR_BUILD=${CC_FOR_BUILD-${CC-gcc}}
else
	CC_FOR_BUILD=${CC_FOR_BUILD-gcc}
fi

AC_SUBST(CC_FOR_BUILD)

AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(LD, ld)

AC_GNULIBC
AC_BSD_SIGNALS

dnl *************************************************************
dnl * Check for required libraries
dnl *************************************************************

AC_CHECK_FUNC([gethostbyname], ,
              [AC_CHECK_LIB([nsl], [gethostbyname], [LIBNSL="-lnsl"])])
AC_SUBST(LIBNSL)

AC_CHECK_FUNC([connect], ,
              [AC_CHECK_LIB([socket], [connect], [LIBSOCKET="-lsocket"],
                    [AC_MSG_ERROR([Function 'socket' not found.])], [$LIBNSL])])

AC_CHECK_FUNC([getaddrinfo], ,
              [AC_MSG_ERROR([Function 'getaddrinfo' not found.])])

AC_CHECK_FUNC([getservbyname], ,
              [AC_MSG_ERROR([Function 'getservbyname' not found.])])

AC_CHECK_LIB([crypt], [crypt], [LIBCRYPT="-lcrypt"])

AC_CHECK_HEADERS([sched.h], [], [])
AC_CHECK_FUNCS([unshare fstatat statx], [] , [])
AC_LIBPTHREAD([])

# rpc/rpc.h can come from the glibc or from libtirpc
nfsutils_save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${CPPFLAGS} ${TIRPC_CFLAGS}"
AC_CHECK_HEADER(rpc/rpc.h, ,
                AC_MSG_ERROR([Header file rpc/rpc.h not found - maybe try building with --enable-tirpc]))
CPPFLAGS="${nfsutils_save_CPPFLAGS}"

dnl check for libevent libraries and headers
AC_LIBEVENT

dnl Check for sqlite3
AC_SQLITE3_VERS

case $libsqlite3_cv_is_recent in
yes) ;;
unknown)
   dnl do not fail when cross-compiling
   AC_MSG_WARN([assuming sqlite is at least v3.3]) ;;
*)
   AC_MSG_ERROR([nfsdcld requires sqlite-devel]) ;;
esac

if test "$enable_nfsv4" = yes; then

  dnl check for the keyutils libraries and headers
  AC_KEYUTILS

  if test "$enable_nfsdcld" = "yes"; then
	AC_CHECK_HEADERS([libgen.h sys/inotify.h], ,
		AC_MSG_ERROR([Cannot find header needed for nfsdcld]))
  fi

  if test "$enable_nfsdcltrack" = "yes"; then
	AC_CHECK_HEADERS([libgen.h sys/inotify.h], ,
		AC_MSG_ERROR([Cannot find header needed for nfsdcltrack]))
  fi

else
  enable_nfsdcld="no"
  enable_nfsdcltrack="no"
fi

if test "$enable_nfsv41" = yes; then
  AC_CHECK_LIB([devmapper], [dm_task_create], [LIBDEVMAPPER="-ldevmapper"], AC_MSG_ERROR([libdevmapper needed]))
  AC_CHECK_HEADER(libdevmapper.h, , AC_MSG_ERROR([Cannot find devmapper header file libdevmapper.h]))
  AC_CHECK_HEADER(sys/inotify.h, , AC_MSG_ERROR([Cannot find header file sys/inotify.h]))
fi

dnl enable nfsidmap when its support by libnfsidmap
AM_CONDITIONAL(CONFIG_NFSDCLD, [test "$enable_nfsdcld" = "yes" ])
AM_CONDITIONAL(CONFIG_NFSDCLTRACK, [test "$enable_nfsdcltrack" = "yes" ])


if test "$knfsd_cv_glibc2" = no; then
    AC_CHECK_LIB(bsd, daemon, [LIBBSD="-lbsd"])
fi

if test "$choose_blkid" != no; then 
   AC_CHECK_LIB(blkid, blkid_get_library_version, [LIBBLKID="-lblkid"], AC_MSG_ERROR([libblkid needed]))
   AC_CHECK_HEADER(blkid/blkid.h, , AC_MSG_ERROR([Cannot find libblkid header file blkid/blkid.h]))
   AC_BLKID_VERS
   if test $choose_blkid = yes; then
      use_blkid=1
      test $libblkid_cv_is_recent = no && AC_MSG_WARN([libblkid is old and may cause mountd to leak memory])
   else
      if test $libblkid_cv_is_recent = yes
      then use_blkid=1
      else use_blkid=0
	   AC_MSG_WARN([uuid support disabled as libblkid is too old])
      fi
    fi
    AC_DEFINE_UNQUOTED(USE_BLKID, $use_blkid, [Define if you want to use blkid to find uuid of filesystems])
fi
AC_SUBST(LIBSOCKET)
AC_SUBST(LIBCRYPT)
AC_SUBST(LIBBSD)
AC_SUBST(LIBBLKID)

if test "$enable_libmount" = yes; then
   AC_CHECK_LIB(mount, mnt_context_do_mount, [LIBMOUNT="-lmount"], AC_MSG_ERROR([libmount needed]))
   AC_CHECK_HEADER(libmount/libmount.h, , AC_MSG_ERROR([Cannot find libmount header file libmount/libmount.h]))
fi
AM_CONDITIONAL(CONFIG_LIBMOUNT, [test "$enable_libmount" = "yes"])
AC_SUBST(LIBMOUNT)

if test "$enable_gss" = yes; then
  dnl 'gss' requires getnameinfo - at least for gssd_proc.c
  AC_CHECK_FUNC([getnameinfo], , [AC_MSG_ERROR([GSSAPI support requires 'getnameinfo' function])])

  dnl check for libevent libraries and headers
  AC_LIBEVENT

  dnl Check for Kerberos V5
  AC_KERBEROS_V5

  dnl Check for pthreads
  AC_LIBPTHREAD([AC_MSG_ERROR([libpthread not found.])])

  dnl librpcsecgss already has a dependency on libgssapi,
  dnl but we need to make sure we get the right version
  if test "$enable_gss" = yes; then
    AC_RPCSEC_VERSION
    if test x"$GSSGLUE_LIBS" != x""; then
      GSSAPI_CFLAGS=$GSSGLUE_CFLAGS
      GSSAPI_LIBS=$GSSGLUE_LIBS
    else
      GSSAPI_CFLAGS=$GSSKRB_CFLAGS
      GSSAPI_LIBS=$GSSKRB_LIBS
    fi
    AC_SUBST([GSSAPI_CFLAGS])
    AC_SUBST([GSSAPI_LIBS])
  fi
fi

dnl libdnsidmap specific checks
AC_CHECK_LIB([resolv], [__res_querydomain], ,
	AC_CHECK_LIB([resolv], [res_querydomain], , AC_MSG_ERROR(res_querydomain needed)))

AC_ARG_ENABLE([ldap],
	[AS_HELP_STRING([--disable-ldap],[Disable support for LDAP @<:default=detect@:>@])])

dnl will libdnsidmap support LDAP ?
if test "x$enable_ldap" != "xno" ; then
        AC_CHECK_HEADER([ldap.h],
                [AC_CHECK_LIB([ldap], [ldap_initialize],
                              [have_ldap="yes"],[have_ldap="no"])],
                [have_ldap="no"])
        if test "x$have_ldap" = "xyes" ; then
                dnl check for sasl funcs
                AC_CHECK_HEADERS(sasl.h sasl/sasl.h)
                AC_CHECK_HEADERS(gsssasl.h)
                AC_CHECK_TYPE(sasl_interact_t,[have_sasl_interact_t="yes"],,[
                        #ifdef HAVE_SASL_SASL_H
                        #include <sasl/sasl.h>
                        #elif defined(HAVE_SASL_H)
                        #include <sasl.h>
                        #endif])
		AC_CHECK_LIB([ldap],[ldap_sasl_interactive_bind_s],[have_ldap_sasl_interactive_bind_s="yes"])
		AC_CHECK_LIB([gssapi_krb5],[gss_krb5_ccache_name],[have_gss_krb5_ccache_name="yes"])
		if test "x$have_sasl_interact_t" = "xyes" -a \
			"x$have_ldap_sasl_interactive_bind_s" = "xyes" -a \
			"x$have_gss_krb5_ccache_name" = "xyes"; then
			AC_DEFINE([HAVE_LDAP_SASL_INTERACTIVE_BIND_S],[1],[Has ldap_sasl_interactive_bind_s function])
			AC_DEFINE([HAVE_GSS_KRB5_CCACHE_NAME],[1],[Has gss_krb5_ccache_name function])
			AC_CHECK_HEADERS(gssapi/gssapi.h gssapi/gssapi_generic.h gssapi/gssapi_krb5.h gssapi.h krb5.h)
			AC_DEFINE([ENABLE_LDAP_SASL],1,[Enable LDAP SASL support])
			have_ldap_sasl="yes"
		fi
		AC_DEFINE([ENABLE_LDAP], 1, [Enable LDAP Support])
        elif test "x$enable_ldap$have_ldap" = "xyesno" ; then
                AC_MSG_ERROR(LDAP support not found!)
        fi
fi
AM_CONDITIONAL(ENABLE_LDAP, test "x$have_ldap" = "xyes")
AM_CONDITIONAL(ENABLE_LDAP_SASL, test "x$have_ldap_sasl" = "xyes")

dnl Should we build gums mapping library?
AC_ARG_ENABLE([gums],
        [AS_HELP_STRING([--enable-gums],[Enable support for the GUMS mapping library @<:@default=false@:>@])])
if test "x$enable_gums" = "xyes" ; then
        AC_DEFINE([ENABLE_GUMS], 1, [Enable GUMS mapping library support])
fi
AM_CONDITIONAL(ENABLE_GUMS, test "x$enable_gums" = "xyes")

dnl Where do the Plugins live
AC_ARG_WITH(pluginpath,
    [AS_HELP_STRING([--with-pluginpath=/foo],[Causes the library to look in /foo instead of /usr/lib/libnfsidmap for plugins
    ])],
    path_plugins=$withval,
    path_plugins=""
    )
if test -n "$path_plugins" ; then
        AC_DEFINE_UNQUOTED(PATH_PLUGINS, "$path_plugins",
                [Define this to change the plugins path])
fi
AM_CONDITIONAL(PATH_PLUGINS, test -n "$path_plugins")
AC_SUBST(PATH_PLUGINS, "$path_plugins")

AC_SUBST(AM_CPPFLAGS, "$AM_CPPFLAGS")
AC_DEFINE([HAVE_NFS4_SET_DEBUG], 1,
          [Bundled lib always has the `nfs4_set_debug' function.])

dnl Check for IPv6 support
AC_IPV6

dnl *************************************************************
dnl Check for headers
dnl *************************************************************
AC_CHECK_HEADERS([arpa/inet.h fcntl.h libintl.h limits.h \
                 malloc.h memory.h netdb.h netinet/in.h paths.h \
                 stdlib.h string.h sys/file.h sys/ioctl.h sys/mount.h \
                 sys/param.h sys/socket.h sys/time.h sys/vfs.h \
                 syslog.h unistd.h com_err.h et/com_err.h \
                 ifaddrs.h])

dnl *************************************************************
dnl Checks for typedefs, structures, and compiler characteristics
dnl *************************************************************
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

AC_STRUCT_TM
AC_CHECK_TYPES([struct file_handle], [], [], [[
		#define _GNU_SOURCE
		#include <sys/types.h>
		#include <sys/stat.h>
		#include <fcntl.h>
	]])

dnl *************************************************************
dnl Check for functions
dnl *************************************************************
AC_FUNC_ALLOCA
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_GETGROUPS
AC_FUNC_GETMNTENT
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_HEADER_MAJOR
#AC_FUNC_MALLOC
AC_FUNC_MEMCMP
#AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES

AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([alarm atexit dup2 fdatasync ftruncate getcwd \
               gethostbyaddr gethostbyname gethostname getmntent \
               getnameinfo getrpcbyname getifaddrs \
               gettimeofday hasmntopt inet_ntoa innetgr memset mkdir pathconf \
               ppoll realpath rmdir select socket strcasecmp strchr strdup \
               strerror strrchr strtol strtoul sigprocmask name_to_handle_at])

save_CFLAGS=$CFLAGS
save_LIBS=$LIBS
CFLAGS="$CFLAGS $AM_CPPFLAGS"
LIBS="$LIBS $LIBTIRPC"
AC_CHECK_FUNCS([getrpcbynumber getrpcbynumber_r])
CFLAGS=$save_CFLAGS
LIBS=$save_LIBS

if test "$ac_cv_func_getrpcbynumber_r" != "yes" -a "$ac_cv_func_getrpcbynumber" != "yes"; then
   AC_MSG_ERROR([Neither getrpcbynumber_r nor getrpcbynumber are available])
fi

dnl *************************************************************
dnl Check for data sizes
dnl *************************************************************
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(size_t)
AC_CHECK_SIZEOF(socklen_t,, [AC_INCLUDES_DEFAULT
                             #ifdef HAVE_SYS_SOCKET_H
                             # include <sys/socket.h>
                             #endif])


dnl *************************************************************
dnl Export some path names to config.h
dnl *************************************************************
AC_DEFINE_UNQUOTED(NFS_STATEDIR, "$statedir", [This defines the location of the NFS state files. Warning: this must match definitions in config.mk!])
AC_DEFINE_UNQUOTED(NSM_DEFAULT_STATEDIR, "$statdpath", [Define this to the pathname where statd keeps its state file])
AC_DEFINE_UNQUOTED(NFS_CONFFILE, "$nfsconfig", [This defines the location of NFS daemon config file])

if test "x$cross_compiling" = "xno"; then
	CFLAGS_FOR_BUILD=${CFLAGS_FOR_BUILD-"$CFLAGS"}
	CXXFLAGS_FOR_BUILD=${CXXFLAGS_FOR_BUILD-"$CXXFLAGS"}
	CPPFLAGS_FOR_BUILD=${CPPFLAGS_FOR_BUILD-"$CPPFLAGS"}
	LDFLAGS_FOR_BUILD=${LDFLAGS_FOR_BUILD-"$LDFLAGS"}
else
	CFLAGS_FOR_BUILD=${CFLAGS_FOR_BUILD-""}
	CXXFLAGS_FOR_BUILD=${CXXFLAGS_FOR_BUILD-""}
	CPPFLAGS_FOR_BUILD=${CPPFLAGS_FOR_BUILD-""}
	LDFLAGS_FOR_BUILD=${LDFLAGS_FOR_BUILD-""}
fi

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

AC_SUBST(CFLAGS_FOR_BUILD)
AC_SUBST(CXXFLAGS_FOR_BUILD)
AC_SUBST(CPPFLAGS_FOR_BUILD)
AC_SUBST(LDFLAGS_FOR_BUILD)

my_am_cflags="\
 -pipe \
 -Wall \
 -Wextra \
 $rpcgen_cflags \
 -Werror=missing-prototypes \
 -Werror=missing-declarations \
 -Werror=format=2 \
 -Werror=undef \
 -Werror=missing-include-dirs \
 -Werror=strict-aliasing=2 \
 -Werror=init-self \
 -Werror=implicit-function-declaration \
 -Werror=return-type \
 -Werror=switch \
 -Werror=overflow \
 -Werror=parentheses \
 -Werror=aggregate-return \
 -Werror=unused-result \
 -fno-strict-aliasing \
"

AC_DEFUN([CHECK_CCSUPPORT], [
  my_save_cflags="$CFLAGS"
  CFLAGS="-Werror $1"
  AC_MSG_CHECKING([whether CC supports $1])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])]
    [$2+=$1],
    [AC_MSG_RESULT([no])]
  )
  CFLAGS="$my_save_cflags"
])

CHECK_CCSUPPORT([-Werror=format-overflow=2], [flg1])
CHECK_CCSUPPORT([-Werror=int-conversion], [flg2])
CHECK_CCSUPPORT([-Werror=incompatible-pointer-types], [flg3])
CHECK_CCSUPPORT([-Werror=misleading-indentation], [flg4])
CHECK_CCSUPPORT([-Wno-cast-function-type], [flg5])
AX_GCC_FUNC_ATTRIBUTE([format])

AC_SUBST([AM_CFLAGS], ["$my_am_cflags $flg1 $flg2 $flg3 $flg4 $flg5"])

# Make sure that $ACLOCAL_FLAGS are used during a rebuild
AC_SUBST([ACLOCAL_AMFLAGS], ["-I $ac_macro_dir \$(ACLOCAL_FLAGS)"])

# make _sysconfdir available for substitution in config files
# 2 "evals" needed late to expand variable names.
AC_SUBST([_sysconfdir])
AC_CONFIG_COMMANDS_PRE([eval eval _sysconfdir=$sysconfdir])

# make _statedir available for substitution in config files
# 2 "evals" needed late to expand variable names.
AC_SUBST([_statedir])
AC_CONFIG_COMMANDS_PRE([eval eval _statedir=$statedir])

if test "$statedir" = "/var/lib/nfs"; then
	rpc_pipefsmount="var-lib-nfs-rpc_pipefs.mount"
else
	rpc_pipefsmount="$(systemd-escape -p "$statedir/rpc_pipefs").mount"
fi
AC_SUBST(rpc_pipefsmount)

# make _rpc_pipefsmount available for substitution in config files
# 2 "evals" needed late to expand variable names.
AC_SUBST([_rpc_pipefsmount])
AC_CONFIG_COMMANDS_PRE([eval eval _rpc_pipefsmount=$rpc_pipefsmount])

AC_CONFIG_FILES([
	Makefile
	systemd/rpc-gssd.service
	systemd/rpc_pipefs.target
	systemd/var-lib-nfs-rpc_pipefs.mount
	linux-nfs/Makefile
	support/Makefile
	support/export/Makefile
	support/include/nfs/Makefile
	support/include/rpcsvc/Makefile
	support/include/sys/fs/Makefile
	support/include/sys/Makefile
	support/include/Makefile
	support/junction/Makefile
	support/misc/Makefile
	support/nfs/Makefile
	support/nsm/Makefile
	support/nfsidmap/Makefile
	support/nfsidmap/libnfsidmap.pc
	support/reexport/Makefile
	tools/Makefile
	tools/locktest/Makefile
	tools/nlmtest/Makefile
	tools/rpcdebug/Makefile
	tools/rpcgen/Makefile
	tools/mountstats/Makefile
	tools/nfs-iostat/Makefile
	tools/nfsrahead/Makefile
	tools/rpcctl/Makefile
	tools/nfsdclnts/Makefile
	tools/nfsconf/Makefile
	tools/nfsdclddb/Makefile
	utils/Makefile
	utils/blkmapd/Makefile
	utils/nfsdcld/Makefile
	utils/nfsdcltrack/Makefile
	utils/exportfs/Makefile
	utils/gssd/Makefile
	utils/idmapd/Makefile
	utils/mount/Makefile
	utils/mountd/Makefile
	utils/exportd/Makefile
	utils/nfsd/Makefile
	utils/nfsref/Makefile
	utils/nfsstat/Makefile
	utils/nfsidmap/Makefile
	utils/showmount/Makefile
	utils/statd/Makefile
	systemd/Makefile
	tests/Makefile
	tests/nsm_client/Makefile])
AC_OUTPUT

