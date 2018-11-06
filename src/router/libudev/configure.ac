AC_PREREQ([2.68])
AC_INIT([eudev],[3.2],[https://github.com/gentoo/eudev/issues])
AC_SUBST(UDEV_VERSION, 220)
AC_CONFIG_SRCDIR([src/udev/udevd.c])

AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_PREFIX_DEFAULT([/usr])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign 1.11])
AM_SILENT_RULES([yes])

LT_PREREQ(2.2)
LT_INIT

# Checks for programs.
AC_PROG_MKDIR_P
AC_PROG_LN_S
AC_PROG_SED
AC_PROG_GREP
AC_PROG_AWK

AC_PROG_CC_C99
AS_IF([test "x$ac_cv_prog_cc_c99" = "xno"], [
        AC_MSG_ERROR([no C99 compiler found, $PACKAGE requires a C99 compiler.])
])

AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_PATH_PROG([M4], [m4])

# Checks for header files.
AC_CHECK_HEADERS(
        [arpa/inet.h fcntl.h inttypes.h langinfo.h limits.h locale.h \
         netinet/in.h sys/ioctl.h sys/mount.h \
         sys/param.h sys/socket.h sys/statvfs.h sys/time.h sys/vfs.h syslog.h \
         termios.h unistd.h],
        [],
        [AC_MSG_ERROR([*** POSIX header not found])]
)

AC_CHECK_HEADERS(
        [mtd/mtd-user.h],
        [],
        [AC_MSG_ERROR([*** KERNEL header not found])]
)

AC_CHECK_HEADERS(
        [linux/btrfs.h],
        [],
        [AC_MSG_WARN([*** KERNEL header not found])]
)

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_CHECK_DECLS([getrandom, gettid, name_to_handle_at, accept4, mkostemp, ppoll, strndupa], [], [],
[[#include <fcntl.h>
#include <linux/random.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>]])

AC_CHECK_SIZEOF(pid_t)
AC_CHECK_SIZEOF(uid_t)
AC_CHECK_SIZEOF(gid_t)
AC_CHECK_SIZEOF(dev_t)
AC_CHECK_SIZEOF(time_t)
AC_CHECK_SIZEOF(rlim_t,,[[
#include <sys/time.h>
#include <sys/resource.h>]])

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_FSEEKO
AC_FUNC_GETGROUPS
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_HEADER_MAJOR
AC_FUNC_MMAP

AC_CHECK_FUNCS(
        [alarm dup2 ftruncate localtime_r mempcpy \
         mkdir munmap nl_langinfo rmdir setlocale socket stpcpy \
         uname],
        [],
        [AC_MSG_ERROR([*** POSIX function not found])]
)
AC_SEARCH_LIBS([clock_gettime], [rt], [], [AC_MSG_ERROR([*** POSIX librt not found])])
LT_LIB_M

# ------------------------------------------------------------------------------

# TODO: the old python checks are irrelevant, but we do need python and perl for tests

# ------------------------------------------------------------------------------
# Set paths here

AC_ARG_WITH(
        [rootprefix],
        [AS_HELP_STRING(
                [--with-rootprefix=DIR],
                [rootfs directory prefix for config files and kernel modules])],
        [],
        [with_rootprefix="\${prefix}"]
)

AC_ARG_WITH(
        [rootlibdir],
        [AS_HELP_STRING(
                [--with-rootlibdir=DIR],
                [Root directory for libraries necessary for boot])],
        [],
        [with_rootlibdir=${libdir}]
)

AC_ARG_WITH(
        [rootlibexecdir],
        [AS_HELP_STRING(
                [--with-rootlibexecdir=DIR],
                [Root directory for libexecs necessary for boot])],
        [],
        [with_rootlibexecdir=${with_rootlibdir}/udev]
)

AC_ARG_WITH(
        [rootrundir],
        [AS_HELP_STRING(
                [--with-rootrundir=DIR],
                [Configurable path for /run])],
        [],
        [with_rootrundir=/run]
)

AC_ARG_ENABLE(
        [split-usr],
        [AS_HELP_STRING(
                [--enable-split-usr],
                [Include hard-coded default search paths in / and /usr])],
        [],
        [AS_IF(
                [test "x${ac_default_prefix}" != "x${with_rootprefix}" && test "x${with_rootprefix}" != "x\${prefix}"],
                [enable_split_usr=yes],
                [enable_split_usr=no])]
)

AS_IF(
        [test "x${enable_split_usr}" = "xyes"],
        [AC_DEFINE(HAVE_SPLIT_USR, 1, [Define to include hard-coded default search paths in / and /usr])]
)

# Configured paths
AC_SUBST([rootprefix], [$with_rootprefix])
AC_SUBST([rootlibdir], [$with_rootlibdir])
AC_SUBST([rootlibexecdir], [${with_rootlibexecdir}])
AC_SUBST([udevlibexecdir], [${rootlibexecdir}])

# sysconfdir paths
AC_SUBST([udevconfdir],[${sysconfdir}/udev])
AC_SUBST([udevconffile],[${udevconfdir}/udev.conf])
AC_SUBST([udevhwdbdir],[${udevconfdir}/hwdb.d])
AC_SUBST([udevhwdbbin],[${udevconfdir}/hwdb.bin])

# udevlibexecdir paths
AC_SUBST([udevkeymapdir],[${udevlibexecdir}/keymaps])
AC_SUBST([udevkeymapforceredir],[${udevkeymapdir}/force-release])
AC_SUBST([udevrulesdir],[${udevlibexecdir}/rules.d])

# pkgconfigdir paths
AC_SUBST([pkgconfiglibdir], [${libdir}/pkgconfig])
AC_SUBST([sharepkgconfigdir],[${datadir}/pkgconfig])

# introspection paths
AC_SUBST([girdir], [${datadir}/gir-1.0])
AC_SUBST([typelibsdir], [${libdir}/girepository-1.0])

AC_SUBST([rootrundir],[${with_rootrundir}])

# ------------------------------------------------------------------------------

GOBJECT_INTROSPECTION_CHECK([1.31.1])
AM_CONDITIONAL([HAVE_INTROSPECTION], [test "$enable_introspection" = "yes"])

# ------------------------------------------------------------------------------
have_blkid=no
AC_ARG_ENABLE(blkid, AS_HELP_STRING([--disable-blkid], [Disable optional blkid support]))
if test "x$enable_blkid" != "xno"; then
        PKG_CHECK_MODULES([BLKID], [blkid >= 2.20],
                [AC_DEFINE(HAVE_BLKID, 1, [Define if blkid is available]) have_blkid=yes], have_blkid=no)
        if test "x$have_blkid" = xno -a "x$enable_blkd" = xyes; then
                AC_MSG_ERROR([*** blkid support requested but not found])
        fi
fi
AM_CONDITIONAL(HAVE_BLKID, [test "x$have_blkid" = "xyes"])

# ------------------------------------------------------------------------------
have_selinux=no
AC_ARG_ENABLE(selinux, AS_HELP_STRING([--disable-selinux], [Disable optional SELINUX support]))
if test "x$enable_selinux" != "xno"; then
        PKG_CHECK_MODULES([SELINUX], [libselinux >= 2.1.9],
                [AC_DEFINE(HAVE_SELINUX, 1, [Define if SELinux is available]) have_selinux=yes], have_selinux=no)
        if test "x$have_selinux" = xno -a "x$enable_selinux" = xyes; then
                AC_MSG_ERROR([*** SELinux support requested but libraries not found])
        fi
fi
AM_CONDITIONAL(HAVE_SELINUX, [test "$have_selinux" = "yes"])
if test "x${have_selinux}" != xno ; then
        sushell=/sbin/sushell
else
        sushell=/bin/bash
fi
AC_SUBST(sushell)

# selinux-util.c uses struct mallinfo which is not available for all C libraries (musl).
AC_CHECK_FUNCS([mallinfo])

# ------------------------------------------------------------------------------

AC_CHECK_DECL([unshare],
        [AC_DEFINE(HAVE_UNSHARE, 1, [Define if unshare is declared])],
        [AC_CHECK_DECL([SYS_unshare],
                [ ] ,
                [AC_MSG_ERROR([*** unshare nor SYS_unshare found.])],
                [#include <syscall.h>])],
        [#include <sched.h>])

# ------------------------------------------------------------------------------
AC_PATH_TOOL(GPERF, gperf)
if test -z "$GPERF" ; then
        AC_MSG_ERROR([*** gperf not found])
fi

# ------------------------------------------------------------------------------
AC_ARG_ENABLE([manpages], AS_HELP_STRING([--disable-manpages],[disable manpages]),[],[enable_manpages=no])
AM_CONDITIONAL(ENABLE_MANPAGES, [test "x$enable_manpages" = "xyes"])

# ------------------------------------------------------------------------------
have_kmod=no
AC_ARG_ENABLE(kmod, AS_HELP_STRING([--disable-kmod], [disable loadable modules support]))
if test "x$enable_kmod" != "xno"; then
        PKG_CHECK_EXISTS([ libkmod ], have_kmod=yes, have_kmod=no)
        if test "x$have_kmod" = "xyes"; then
                PKG_CHECK_MODULES(KMOD, [ libkmod >= 15 ],
                        [AC_DEFINE(HAVE_KMOD, 1, [Define if kmod is available])],
                        AC_MSG_ERROR([*** kmod version >= 15 not found]))
        fi
        if test "x$have_kmod" = xno -a "x$enable_kmod" = xyes; then
                AC_MSG_ERROR([*** kmod support requested, but libraries not found])
        fi
fi
AM_CONDITIONAL(HAVE_KMOD, [test "$have_kmod" = "yes"])

# ------------------------------------------------------------------------------

AC_ARG_ENABLE([hwdb], AS_HELP_STRING([--enable-hwdb],[install hwdb.d files]),[],[enable_hwdb=yes])
AM_CONDITIONAL(ENABLE_HWDB, [test "x$enable_hwdb" = "xyes"])

# ------------------------------------------------------------------------------
# rule-generator - persistent network and optical device rule generator
# ------------------------------------------------------------------------------
AC_ARG_ENABLE([rule-generator],
        AS_HELP_STRING([--enable-rule-generator], [enable legacy persistent network, cdrom support]),
        [], [enable_rule_generator=no])

if test "x${enable_rule_generator}" != xno; then
        AC_DEFINE([ENABLE_RULE_GENERATOR], [1], [Define if we are enabling rule generator])
fi

AM_CONDITIONAL([ENABLE_RULE_GENERATOR], [test "x$enable_rule_generator" = xyes])

# ------------------------------------------------------------------------------

AC_CONFIG_FILES([Makefile
                 hwdb/Makefile
                 man/Makefile
                 rule_generator/Makefile
                 rule_generator/write_net_rules
                 rules/Makefile
                 src/Makefile
                 src/ata_id/Makefile
                 src/cdrom_id/Makefile
                 src/collect/Makefile
                 src/mtd_probe/Makefile
                 src/scsi_id/Makefile
                 src/v4l_id/Makefile
                 src/shared/Makefile
                 src/libudev/Makefile
                 src/libudev/libudev.pc
                 src/udev/Makefile
                 src/udev/udev.pc
                 test/Makefile])

AC_OUTPUT

# ------------------------------------------------------------------------------

AC_MSG_RESULT([
        prefix:                  ${prefix}
        exec_prefix:             ${exec_prefix}
        sysconfdir:              ${sysconfdir}
        datadir:                 ${datadir}
        includedir:              ${includedir}
        libdir:                  ${libdir}

        rootprefix:              ${rootprefix}
        rootlibdir:              ${rootlibdir}
        rootlibexecdir:          ${rootlibexecdir}
        datarootdir:             ${datarootdir}
        rootrundir:              ${rootrundir}

        udevconfdir:             ${udevconfdir}
        udevconffile:            ${udevconffile}
        udevhwdbdir:             ${udevhwdbdir}
        udevhwdbbin:             ${udevhwdbbin}
        udevlibexecdir:          ${udevlibexecdir}
        udevkeymapdir:           ${udevkeymapdir}
        udevkeymapforceredir:    ${udevkeymapforceredir}
        udevrulesdir:            ${udevrulesdir}

        pkgconfiglibdir:         ${libdir}/pkgconfig
        sharepkgconfigdir        ${datadir}/pkgconfig

        girdir                   ${datadir}/gir-1.0
        typelibsdir              ${libdir}/girepository-1.0
])

# ------------------------------------------------------------------------------

dnl Set configured scripts executable
if test -f  src/keymap/check-keymaps.sh; then
        chmod +x src/keymap/check-keymaps.sh
fi

if test -f src/keymap/keyboard-force-release.sh; then
        chmod +x src/keymap/keyboard-force-release.sh
fi

