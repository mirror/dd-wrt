###############################################################################
## Copyright (C) 2000-2004 Sistina Software, Inc. All rights reserved.
## Copyright (C) 2004-2016 Red Hat, Inc. All rights reserved.
##
## This copyrighted material is made available to anyone wishing to use,
## modify, copy, or redistribute it subject to the terms and conditions
## of the GNU General Public License v.2.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software Foundation,
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
################################################################################

AC_PREREQ(2.69)
################################################################################
dnl -- Process this file with autoconf to produce a configure script.
AC_INIT
CONFIGURE_LINE="$0 $@"
AC_CONFIG_SRCDIR([lib/device/dev-cache.h])
AC_CONFIG_HEADERS([include/configure.h])

################################################################################
dnl -- Setup the directory where autoconf has auxilary files
AC_CONFIG_AUX_DIR(autoconf)

################################################################################
dnl -- Get system type
AC_CANONICAL_TARGET([])

AS_IF([test -z "$CFLAGS"], [COPTIMISE_FLAG="-O2"])
case "$host_os" in
	linux*)
		CLDFLAGS="${CLDFLAGS:"$LDFLAGS"} -Wl,--version-script,.export.sym"
		# equivalent to -rdynamic
		ELDFLAGS="-Wl,--export-dynamic"
		# FIXME Generate list and use --dynamic-list=.dlopen.sym
		CLDWHOLEARCHIVE="-Wl,-whole-archive"
		CLDNOWHOLEARCHIVE="-Wl,-no-whole-archive"
		LDDEPS="$LDDEPS .export.sym"
		LIB_SUFFIX=so
		DEVMAPPER=yes
		BUILD_LVMPOLLD=no
		LOCKDSANLOCK=no
		LOCKDDLM=no
		ODIRECT=yes
		DM_IOCTLS=yes
		SELINUX=yes
		FSADM=yes
		BLKDEACTIVATE=yes
		;;
	darwin*)
		CFLAGS="$CFLAGS -no-cpp-precomp -fno-common"
		CLDFLAGS="${CLDFLAGS:"$LDFLAGS"}"
		ELDFLAGS=
		CLDWHOLEARCHIVE="-all_load"
		CLDNOWHOLEARCHIVE=
		LIB_SUFFIX=dylib
		DEVMAPPER=yes
		ODIRECT=no
		DM_IOCTLS=no
		SELINUX=no
		FSADM=no
		BLKDEACTIVATE=no
		;;
	*)
		CLDFLAGS="${CLDFLAGS:"$LDFLAGS"}"
		;;
esac

################################################################################
dnl -- Checks for programs.
AC_PROG_SED
AC_PROG_AWK
save_CFLAGS=$CFLAGS
save_CXXFLAGS=$CXXFLAGS
AC_PROG_CC
AC_PROG_CXX
CFLAGS=$save_CFLAGS
CXXFLAGS=$save_CXXFLAGS
PATH_SBIN="$PATH:/usr/sbin:/sbin"

dnl probably no longer needed in 2008, but...
AC_PROG_GCC_TRADITIONAL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_RANLIB
AC_CHECK_TOOL(AR, ar)
AC_PATH_TOOL(CFLOW_CMD, cflow)
AC_PATH_TOOL(CSCOPE_CMD, cscope)
AC_PATH_TOOL(CHMOD, chmod)
AC_PATH_TOOL(WC, wc)
AC_PATH_TOOL(SORT, sort)

################################################################################
dnl -- Check for header files.
AC_HEADER_DIRENT
AC_HEADER_MAJOR
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME

AC_CHECK_HEADERS([assert.h ctype.h dirent.h errno.h fcntl.h float.h \
  getopt.h inttypes.h langinfo.h libaio.h libgen.h limits.h locale.h paths.h \
  signal.h stdarg.h stddef.h stdio.h stdlib.h string.h sys/file.h \
  sys/ioctl.h syslog.h sys/mman.h sys/param.h sys/resource.h sys/stat.h \
  sys/time.h sys/types.h sys/utsname.h sys/wait.h time.h \
  unistd.h], , [AC_MSG_ERROR(bailing out)])

AC_CHECK_HEADERS(termios.h sys/statvfs.h sys/timerfd.h sys/vfs.h linux/magic.h linux/fiemap.h)

case "$host_os" in
	linux*)
		AC_CHECK_HEADERS(asm/byteorder.h linux/fs.h malloc.h,,AC_MSG_ERROR(bailing out)) ;;
	darwin*)
		AC_CHECK_HEADERS(machine/endian.h sys/disk.h,,AC_MSG_ERROR(bailing out)) ;;
esac

################################################################################
dnl -- Check for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_CHECK_TYPES([ptrdiff_t])
AC_STRUCT_ST_BLOCKS
AC_STRUCT_TM
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_MODE_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AX_GCC_BUILTIN([__builtin_clz])
AX_GCC_BUILTIN([__builtin_clzll])


AC_DEFINE([_GNU_SOURCE], 1, [Define to get access to GNU/Linux extension])
AC_DEFINE([_REENTRANT], 1, [Define to use re-entrant thread safe versions])


################################################################################
dnl -- Check for functions
AC_CHECK_FUNCS([ftruncate gethostname getpagesize gettimeofday localtime_r \
  memchr memset mkdir mkfifo munmap nl_langinfo realpath rmdir setenv \
  setlocale strcasecmp strchr strcspn strdup strerror strncasecmp strndup \
  strrchr strspn strstr strtol strtoul uname], , [AC_MSG_ERROR(bailing out)])
AC_FUNC_ALLOCA
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_LSTAT
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_VPRINTF

################################################################################
dnl -- Disable dependency tracking
AC_MSG_CHECKING(whether to enable dependency tracking)
AC_ARG_ENABLE(dependency-tracking,
	      AC_HELP_STRING([--disable-dependency-tracking],
			     [speeds up one-time build.]),
	      USE_TRACKING=$enableval, USE_TRACKING=yes)
AC_MSG_RESULT($USE_TRACKING)

################################################################################
dnl -- Disable silence rules
AC_MSG_CHECKING(whether to build silently)
AC_ARG_ENABLE(silent-rules,
	      AC_HELP_STRING([--disable-silent-rules], [disable silent building]),
	      SILENT_RULES=$enableval, SILENT_RULES=yes)
AC_MSG_RESULT($SILENT_RULES)


################################################################################
dnl -- Enables statically-linked tools
AC_MSG_CHECKING(whether to use static linking)
AC_ARG_ENABLE(static_link,
	      AC_HELP_STRING([--enable-static_link],
			     [use this to link the tools to their libraries
			      statically (default is dynamic linking]),
	      STATIC_LINK=$enableval, STATIC_LINK=no)
AC_MSG_RESULT($STATIC_LINK)

################################################################################
dnl -- Check if compiler/linker supports PIE and RELRO
AC_TRY_CCFLAG([-pie], [HAVE_PIE], [], [])
AC_SUBST(HAVE_PIE)
AC_TRY_LDFLAGS([-Wl,-z,relro,-z,now], [HAVE_FULL_RELRO], [], [])
AC_SUBST(HAVE_FULL_RELRO)

################################################################################
dnl -- Prefix is /usr by default, the exec_prefix default is setup later
AC_PREFIX_DEFAULT(/usr)

################################################################################
dnl -- Clear default exec_prefix - install into /sbin rather than /usr/sbin
test "$exec_prefix" = NONE -a "$prefix" = NONE && exec_prefix=""

test "x$prefix" = xNONE && prefix=$ac_default_prefix
# Let make expand exec_prefix.
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'


################################################################################
dnl -- Setup the ownership of the files
AC_MSG_CHECKING(file owner)
AC_ARG_WITH(user,
	    AC_HELP_STRING([--with-user=USER],
			   [set the owner of installed files [USER=]]),
	    OWNER=$withval)
AC_MSG_RESULT($OWNER)
test -n "$OWNER" && INSTALL="$INSTALL -o $OWNER"

################################################################################
dnl -- Setup the group ownership of the files
AC_MSG_CHECKING(group owner)
AC_ARG_WITH(group,
	    AC_HELP_STRING([--with-group=GROUP],
			   [set the group owner of installed files [GROUP=]]),
	    GROUP=$withval)
AC_MSG_RESULT($GROUP)
test -n "$GROUP" && INSTALL="$INSTALL -g $GROUP"

################################################################################
dnl -- Setup device node ownership
AC_MSG_CHECKING(device node uid)

AC_ARG_WITH(device-uid,
	    AC_HELP_STRING([--with-device-uid=UID],
			   [set the owner used for new device nodes [UID=0]]),
	    DM_DEVICE_UID=$withval, DM_DEVICE_UID=0)
AC_MSG_RESULT($DM_DEVICE_UID)
AC_DEFINE_UNQUOTED([DM_DEVICE_UID], [$DM_DEVICE_UID], [Define default owner for device node])

################################################################################
dnl -- Setup device group ownership
AC_MSG_CHECKING(device node gid)

AC_ARG_WITH(device-gid,
	    AC_HELP_STRING([--with-device-gid=GID],
			   [set the group used for new device nodes [GID=0]]),
	    DM_DEVICE_GID=$withval, DM_DEVICE_GID=0)
AC_MSG_RESULT($DM_DEVICE_GID)
AC_DEFINE_UNQUOTED([DM_DEVICE_GID], [$DM_DEVICE_GID], [Define default group for device node])

################################################################################
dnl -- Setup device mode
AC_MSG_CHECKING(device node mode)

AC_ARG_WITH(device-mode,
	    AC_HELP_STRING([--with-device-mode=MODE],
			   [set the mode used for new device nodes [MODE=0600]]),
	    DM_DEVICE_MODE=$withval, DM_DEVICE_MODE=0600)
AC_MSG_RESULT($DM_DEVICE_MODE)
AC_DEFINE_UNQUOTED([DM_DEVICE_MODE], [$DM_DEVICE_MODE], [Define default mode for device node])

AC_MSG_CHECKING(when to create device nodes)
AC_ARG_WITH(device-nodes-on,
	    AC_HELP_STRING([--with-device-nodes-on=ON],
			   [create nodes on resume or create [ON=resume]]),
	    ADD_NODE=$withval, ADD_NODE=resume)
case "$ADD_NODE" in
 resume) add_on=DM_ADD_NODE_ON_RESUME;;
 create) add_on=DM_ADD_NODE_ON_CREATE;;
 *) AC_MSG_ERROR([--with-device-nodes-on parameter invalid]);;
esac
AC_MSG_RESULT(on $ADD_NODE)
AC_DEFINE_UNQUOTED([DEFAULT_DM_ADD_NODE], $add_on, [Define default node creation behavior with dmsetup create])

AC_MSG_CHECKING(default name mangling)
AC_ARG_WITH(default-name-mangling,
	    AC_HELP_STRING([--with-default-name-mangling=MANGLING],
			   [default name mangling: auto/none/hex [auto]]),
	    MANGLING=$withval, MANGLING=auto)
case "$MANGLING" in
 auto) mangling=DM_STRING_MANGLING_AUTO;;
 none|disabled) mangling=DM_STRING_MANGLING_NONE;;
 hex) mangling=DM_STRING_MANGLING_HEX;;
 *) AC_MSG_ERROR([--with-default-name-mangling parameter invalid]);;
esac
AC_MSG_RESULT($MANGLING)
AC_DEFINE_UNQUOTED([DEFAULT_DM_NAME_MANGLING], $mangling, [Define default name mangling behaviour])

################################################################################
dnl -- snapshots inclusion type
AC_MSG_CHECKING(whether to include snapshots)
AC_ARG_WITH(snapshots,
	    AC_HELP_STRING([--with-snapshots=TYPE],
			   [snapshot support: internal/shared/none [internal]]),
	    SNAPSHOTS=$withval, SNAPSHOTS=internal)
AC_MSG_RESULT($SNAPSHOTS)

case "$SNAPSHOTS" in
  none|shared) ;;
  internal) AC_DEFINE([SNAPSHOT_INTERNAL], 1,
		      [Define to 1 to include built-in support for snapshots.]) ;;
  *) AC_MSG_ERROR([--with-snapshots parameter invalid]) ;;
esac

################################################################################
dnl -- mirrors inclusion type
AC_MSG_CHECKING(whether to include mirrors)
AC_ARG_WITH(mirrors,
	    AC_HELP_STRING([--with-mirrors=TYPE],
			   [mirror support: internal/shared/none [internal]]),
	    MIRRORS=$withval, MIRRORS=internal)
AC_MSG_RESULT($MIRRORS)

case "$MIRRORS" in
  none|shared) ;;
  internal) AC_DEFINE([MIRRORED_INTERNAL], 1,
		      [Define to 1 to include built-in support for mirrors.]) ;;
  *) AC_MSG_ERROR([--with-mirrors parameter invalid]) ;;
esac

################################################################################
dnl -- raid inclusion type
AC_ARG_WITH(default-mirror-segtype,
	    AC_HELP_STRING([--with-default-mirror-segtype=TYPE],
			   [default mirror segtype: raid1/mirror [raid1]]),
			   DEFAULT_MIRROR_SEGTYPE=$withval, DEFAULT_MIRROR_SEGTYPE="raid1")
AC_ARG_WITH(default-raid10-segtype,
	    AC_HELP_STRING([--with-default-raid10-segtype=TYPE],
			   [default mirror segtype: raid10/mirror [raid10]]),
			   DEFAULT_RAID10_SEGTYPE=$withval, DEFAULT_RAID10_SEGTYPE="raid10")

AC_DEFINE([RAID_INTERNAL], 1,
	  [Define to 1 to include built-in support for raid.])

AC_DEFINE_UNQUOTED([DEFAULT_MIRROR_SEGTYPE], ["$DEFAULT_MIRROR_SEGTYPE"],
		   [Default segtype used for mirror volumes.])

AC_DEFINE_UNQUOTED([DEFAULT_RAID10_SEGTYPE], ["$DEFAULT_RAID10_SEGTYPE"],
		   [Default segtype used for raid10 volumes.])

################################################################################
AC_ARG_WITH(default-sparse-segtype,
	    AC_HELP_STRING([--with-default-sparse-segtype=TYPE],
			   [default sparse segtype: thin/snapshot [thin]]),
			   DEFAULT_SPARSE_SEGTYPE=$withval, DEFAULT_SPARSE_SEGTYPE="thin")

################################################################################
dnl -- thin provisioning
AC_MSG_CHECKING(whether to include thin provisioning)
AC_ARG_WITH(thin,
	    AC_HELP_STRING([--with-thin=TYPE],
			   [thin provisioning support: internal/shared/none [internal]]),
	    THIN=$withval, THIN=internal)
AC_ARG_WITH(thin-check,
	    AC_HELP_STRING([--with-thin-check=PATH],
			   [thin_check tool: [autodetect]]),
			   THIN_CHECK_CMD=$withval, THIN_CHECK_CMD="autodetect")
AC_ARG_WITH(thin-dump,
	    AC_HELP_STRING([--with-thin-dump=PATH],
			   [thin_dump tool: [autodetect]]),
			   THIN_DUMP_CMD=$withval, THIN_DUMP_CMD="autodetect")
AC_ARG_WITH(thin-repair,
	    AC_HELP_STRING([--with-thin-repair=PATH],
			   [thin_repair tool: [autodetect]]),
			   THIN_REPAIR_CMD=$withval, THIN_REPAIR_CMD="autodetect")
AC_ARG_WITH(thin-restore,
	    AC_HELP_STRING([--with-thin-restore=PATH],
			   [thin_restore tool: [autodetect]]),
			   THIN_RESTORE_CMD=$withval, THIN_RESTORE_CMD="autodetect")

AC_MSG_RESULT($THIN)

case "$THIN" in
  none)  test "$DEFAULT_SPARSE_SEGTYPE" = "thin" && DEFAULT_SPARSE_SEGTYPE="snapshot" ;;
  shared) ;;
  internal) AC_DEFINE([THIN_INTERNAL], 1,
		      [Define to 1 to include built-in support for thin provisioning.]) ;;
  *) AC_MSG_ERROR([--with-thin parameter invalid ($THIN)]) ;;
esac

AC_DEFINE_UNQUOTED([DEFAULT_SPARSE_SEGTYPE], ["$DEFAULT_SPARSE_SEGTYPE"],
		   [Default segtype used for sparse volumes.])

dnl -- thin_check needs-check flag
AC_ARG_ENABLE(thin_check_needs_check,
	      AC_HELP_STRING([--disable-thin_check_needs_check],
			     [required if thin_check version is < 0.3.0]),
	      THIN_CHECK_NEEDS_CHECK=$enableval, THIN_CHECK_NEEDS_CHECK=yes)

# Test if necessary thin tools are available
# if not - use plain defaults and warn user
case "$THIN" in
  internal|shared)
	# Empty means a config way to ignore thin checking
	if test "$THIN_CHECK_CMD" = "autodetect"; then
		AC_PATH_TOOL(THIN_CHECK_CMD, thin_check, [], [$PATH_SBIN])
		if test -z "$THIN_CHECK_CMD"; then
			AC_MSG_WARN([thin_check not found in path $PATH])
			THIN_CHECK_CMD=/usr/sbin/thin_check
			THIN_CONFIGURE_WARN=y
		fi
	fi
	if test "$THIN_CHECK_NEEDS_CHECK" = yes; then
		THIN_CHECK_VSN=`"$THIN_CHECK_CMD" -V 2>/dev/null`
		THIN_CHECK_VSN_MAJOR=`echo "$THIN_CHECK_VSN" | $AWK -F '.' '{print $1}'`
		THIN_CHECK_VSN_MINOR=`echo "$THIN_CHECK_VSN" | $AWK -F '.' '{print $2}'`

		if test -z "$THIN_CHECK_VSN_MAJOR" -o -z "$THIN_CHECK_VSN_MINOR"; then
			AC_MSG_WARN([$THIN_CHECK_CMD: Bad version "$THIN_CHECK_VSN" found])
			THIN_CHECK_VERSION_WARN=y
			THIN_CHECK_NEEDS_CHECK=no
		elif test "$THIN_CHECK_VSN_MAJOR" -eq 0 -a "$THIN_CHECK_VSN_MINOR" -lt 3; then
			AC_MSG_WARN([$THIN_CHECK_CMD: Old version "$THIN_CHECK_VSN" found])
			THIN_CHECK_VERSION_WARN=y
			THIN_CHECK_NEEDS_CHECK=no
		fi
	fi
	# Empty means a config way to ignore thin dumping
	if test "$THIN_DUMP_CMD" = "autodetect"; then
		AC_PATH_TOOL(THIN_DUMP_CMD, thin_dump, [], [$PATH_SBIN])
		test -z "$THIN_DUMP_CMD" && {
			AC_MSG_WARN(thin_dump not found in path $PATH)
			THIN_DUMP_CMD=/usr/sbin/thin_dump
			THIN_CONFIGURE_WARN=y
		}
	fi
	# Empty means a config way to ignore thin repairing
	if test "$THIN_REPAIR_CMD" = "autodetect"; then
		AC_PATH_TOOL(THIN_REPAIR_CMD, thin_repair, [], [$PATH_SBIN])
		test -z "$THIN_REPAIR_CMD" && {
			AC_MSG_WARN(thin_repair not found in path $PATH)
			THIN_REPAIR_CMD=/usr/sbin/thin_repair
			THIN_CONFIGURE_WARN=y
		}
	fi
	# Empty means a config way to ignore thin restoring
	if test "$THIN_RESTORE_CMD" = "autodetect"; then
		AC_PATH_TOOL(THIN_RESTORE_CMD, thin_restore, [], [$PATH_SBIN])
		test -z "$THIN_RESTORE_CMD" && {
			AC_MSG_WARN(thin_restore not found in path $PATH)
			THIN_RESTORE_CMD=/usr/sbin/thin_restore
			THIN_CONFIGURE_WARN=y
		}
	fi

	AC_MSG_CHECKING([whether thin_check supports the needs-check flag])
	AC_MSG_RESULT([$THIN_CHECK_NEEDS_CHECK])
	if test "$THIN_CHECK_NEEDS_CHECK" = yes; then
		AC_DEFINE([THIN_CHECK_NEEDS_CHECK], 1, [Define to 1 if the external 'thin_check' tool requires the --clear-needs-check-flag option])
	fi

	;;
esac

AC_DEFINE_UNQUOTED([THIN_CHECK_CMD], ["$THIN_CHECK_CMD"],
		   [The path to 'thin_check', if available.])

AC_DEFINE_UNQUOTED([THIN_DUMP_CMD], ["$THIN_DUMP_CMD"],
		   [The path to 'thin_dump', if available.])

AC_DEFINE_UNQUOTED([THIN_REPAIR_CMD], ["$THIN_REPAIR_CMD"],
		   [The path to 'thin_repair', if available.])

AC_DEFINE_UNQUOTED([THIN_RESTORE_CMD], ["$THIN_RESTORE_CMD"],
		   [The path to 'thin_restore', if available.])

################################################################################
dnl -- cache inclusion type
AC_MSG_CHECKING(whether to include cache)
AC_ARG_WITH(cache,
	    AC_HELP_STRING([--with-cache=TYPE],
			   [cache support: internal/shared/none [internal]]),
			   CACHE=$withval, CACHE="internal")
AC_ARG_WITH(cache-check,
	    AC_HELP_STRING([--with-cache-check=PATH],
			   [cache_check tool: [autodetect]]),
			   CACHE_CHECK_CMD=$withval, CACHE_CHECK_CMD="autodetect")
AC_ARG_WITH(cache-dump,
	    AC_HELP_STRING([--with-cache-dump=PATH],
			   [cache_dump tool: [autodetect]]),
			   CACHE_DUMP_CMD=$withval, CACHE_DUMP_CMD="autodetect")
AC_ARG_WITH(cache-repair,
	    AC_HELP_STRING([--with-cache-repair=PATH],
			   [cache_repair tool: [autodetect]]),
			   CACHE_REPAIR_CMD=$withval, CACHE_REPAIR_CMD="autodetect")
AC_ARG_WITH(cache-restore,
	    AC_HELP_STRING([--with-cache-restore=PATH],
			   [cache_restore tool: [autodetect]]),
			   CACHE_RESTORE_CMD=$withval, CACHE_RESTORE_CMD="autodetect")
AC_MSG_RESULT($CACHE)

case "$CACHE" in
 none|shared) ;;
 internal) AC_DEFINE([CACHE_INTERNAL], 1, [Define to 1 to include built-in support for cache.]) ;;
 *) AC_MSG_ERROR([--with-cache parameter invalid]) ;;
esac

dnl -- cache_check needs-check flag
AC_ARG_ENABLE(cache_check_needs_check,
	      AC_HELP_STRING([--disable-cache_check_needs_check],
			     [required if cache_check version is < 0.5]),
	      CACHE_CHECK_NEEDS_CHECK=$enableval, CACHE_CHECK_NEEDS_CHECK=yes)

# Test if necessary cache tools are available
# if not - use plain defaults and warn user
case "$CACHE" in
  internal|shared)
	# Empty means a config way to ignore cache checking
	if test "$CACHE_CHECK_CMD" = "autodetect"; then
		AC_PATH_TOOL(CACHE_CHECK_CMD, cache_check, [], [$PATH_SBIN])
		if test -z "$CACHE_CHECK_CMD"; then
			AC_MSG_WARN([cache_check not found in path $PATH])
			CACHE_CHECK_CMD=/usr/sbin/cache_check
			CACHE_CONFIGURE_WARN=y
		fi
	fi
	if test "$CACHE_CHECK_NEEDS_CHECK" = yes; then
		$CACHE_CHECK_CMD -V 2>/dev/null >conftest.tmp
		read -r CACHE_CHECK_VSN < conftest.tmp
		IFS=.- read -r CACHE_CHECK_VSN_MAJOR CACHE_CHECK_VSN_MINOR CACHE_CHECK_VSN_PATCH LEFTOVER < conftest.tmp
		rm -f conftest.tmp

		# Require version >= 0.5.4 for --clear-needs-check-flag
		if test -z "$CACHE_CHECK_VSN_MAJOR" \
			|| test -z "$CACHE_CHECK_VSN_MINOR" \
			|| test -z "$CACHE_CHECK_VSN_PATCH"; then
			AC_MSG_WARN([$CACHE_CHECK_CMD: Bad version "$CACHE_CHECK_VSN" found])
			CACHE_CHECK_VERSION_WARN=y
			CACHE_CHECK_NEEDS_CHECK=no
		elif test "$CACHE_CHECK_VSN_MAJOR" -eq 0 ; then
			if test "$CACHE_CHECK_VSN_MINOR" -lt 5 \
				|| test "$CACHE_CHECK_VSN_MINOR" -eq 5 -a "$CACHE_CHECK_VSN_PATCH" -lt 4; then
				AC_MSG_WARN([$CACHE_CHECK_CMD: Old version "$CACHE_CHECK_VSN" found])
				CACHE_CHECK_VERSION_WARN=y
				CACHE_CHECK_NEEDS_CHECK=no
			fi
			if test "$CACHE_CHECK_VSN_MINOR" -lt 7 ; then
				AC_MSG_WARN([$CACHE_CHECK_CMD: Old version "$CACHE_CHECK_VSN" does not support new cache format V2])
				CACHE_CHECK_VERSION_WARN=y
			fi
		fi
	fi
	# Empty means a config way to ignore cache dumping
	if test "$CACHE_DUMP_CMD" = "autodetect"; then
		AC_PATH_TOOL(CACHE_DUMP_CMD, cache_dump, [], [$PATH_SBIN])
		test -z "$CACHE_DUMP_CMD" && {
			AC_MSG_WARN(cache_dump not found in path $PATH)
			CACHE_DUMP_CMD=/usr/sbin/cache_dump
			CACHE_CONFIGURE_WARN=y
		}
	fi
	# Empty means a config way to ignore cache repairing
	if test "$CACHE_REPAIR_CMD" = "autodetect"; then
		AC_PATH_TOOL(CACHE_REPAIR_CMD, cache_repair, [], [$PATH_SBIN])
		test -z "$CACHE_REPAIR_CMD" && {
			AC_MSG_WARN(cache_repair not found in path $PATH)
			CACHE_REPAIR_CMD=/usr/sbin/cache_repair
			CACHE_CONFIGURE_WARN=y
		}
	fi
	# Empty means a config way to ignore cache restoring
	if test "$CACHE_RESTORE_CMD" = "autodetect"; then
		AC_PATH_TOOL(CACHE_RESTORE_CMD, cache_restore, [], [$PATH_SBIN])
		test -z "$CACHE_RESTORE_CMD" && {
			AC_MSG_WARN(cache_restore not found in path $PATH)
			CACHE_RESTORE_CMD=/usr/sbin/cache_restore
			CACHE_CONFIGURE_WARN=y
		}
	fi

	AC_MSG_CHECKING([whether cache_check supports the needs-check flag])
	AC_MSG_RESULT([$CACHE_CHECK_NEEDS_CHECK])
	if test "$CACHE_CHECK_NEEDS_CHECK" = yes; then
		AC_DEFINE([CACHE_CHECK_NEEDS_CHECK], 1, [Define to 1 if the external 'cache_check' tool requires the --clear-needs-check-flag option])
	fi
	;;
esac

AC_DEFINE_UNQUOTED([CACHE_CHECK_CMD], ["$CACHE_CHECK_CMD"],
		   [The path to 'cache_check', if available.])

AC_DEFINE_UNQUOTED([CACHE_DUMP_CMD], ["$CACHE_DUMP_CMD"],
		   [The path to 'cache_dump', if available.])

AC_DEFINE_UNQUOTED([CACHE_REPAIR_CMD], ["$CACHE_REPAIR_CMD"],
		   [The path to 'cache_repair', if available.])

AC_DEFINE_UNQUOTED([CACHE_RESTORE_CMD], ["$CACHE_RESTORE_CMD"],
		   [The path to 'cache_restore', if available.])

################################################################################
dnl -- cache inclusion type
AC_MSG_CHECKING(whether to include vdo)
AC_ARG_WITH(vdo,
	    AC_HELP_STRING([--with-vdo=TYPE],
			   [vdo support: internal/none [internal]]),
			   VDO=$withval, VDO="none")

AC_MSG_RESULT($VDO)

AC_ARG_WITH(vdo-format,
	    AC_HELP_STRING([--with-vdo-format=PATH],
			   [vdoformat tool: [autodetect]]),
			   VDO_FORMAT_CMD=$withval, VDO_FORMAT_CMD="autodetect")
case "$VDO" in
 none) ;;
 internal) 
	AC_DEFINE([VDO_INTERNAL], 1, [Define to 1 to include built-in support for vdo.])
	if test "$VDO_FORMAT_CMD" = "autodetect"; then
		AC_PATH_TOOL(VDO_FORMAT_CMD, vdoformat, [], [$PATH])
		if test -z "$VDO_FORMAT_CMD"; then
			AC_MSG_WARN([vdoformat not found in path $PATH])
			VDO_FORMAT_CMD=/usr/bin/vdoformat
			VDO_CONFIGURE_WARN=y
		fi
	fi
	;;
 *) AC_MSG_ERROR([--with-vdo parameter invalid]) ;;
esac

AC_DEFINE_UNQUOTED([VDO_FORMAT_CMD], ["$VDO_FORMAT_CMD"],
		   [The path to 'vdoformat', if available.])
#
# Do we need to use the API??
# Do we want to link lvm2 with a big library for vdoformating ?
#
#AC_ARG_WITH(vdo-include,
#	    AC_HELP_STRING([--with-vdo-include=PATH],
#			   [vdo support: Path to utils headers: [/usr/include/vdo/utils]]),
#			   VDO_INCLUDE=$withval, VDO_INCLUDE="/usr/include/vdo/utils")
#AC_MSG_RESULT($VDO_INCLUDE)
#
#AC_ARG_WITH(vdo-lib,
#            AC_HELP_STRING([--with-vdo-lib=PATH],
#                           [vdo support: Path to utils lib: [/usr/lib]]),
#                           VDO_LIB=$withval, VDO_LIB="/usr/lib") 
#AC_MSG_RESULT($VDO_LIB)

################################################################################
dnl -- Disable readline
AC_ARG_ENABLE([readline],
	      AC_HELP_STRING([--disable-readline], [disable readline support]),
	      READLINE=$enableval, READLINE=maybe)

################################################################################
dnl -- Disable realtime clock support
AC_MSG_CHECKING(whether to enable realtime support)
AC_ARG_ENABLE(realtime,
	      AC_HELP_STRING([--disable-realtime], [disable realtime clock support]),
	      REALTIME=$enableval, REALTIME=yes)
AC_MSG_RESULT($REALTIME)

################################################################################
dnl -- disable OCF resource agents
AC_MSG_CHECKING(whether to enable OCF resource agents)
AC_ARG_ENABLE(ocf,
	      AC_HELP_STRING([--enable-ocf],
			     [enable Open Cluster Framework (OCF) compliant resource agents]),
	      OCF=$enableval, OCF=no)
AC_MSG_RESULT($OCF)
AC_ARG_WITH(ocfdir,
	    AC_HELP_STRING([--with-ocfdir=DIR],
			   [install OCF files in [PREFIX/lib/ocf/resource.d/lvm2]]),
	    OCFDIR=$withval, OCFDIR='${prefix}/lib/ocf/resource.d/lvm2')

################################################################################
dnl -- Init pkg-config with dummy invokation:
dnl -- this is required because PKG_CHECK_MODULES macro is expanded
dnl -- to initialize the pkg-config environment only at the first invokation,
dnl -- that would be conditional in this configure.in.
pkg_config_init() {
	if test "$PKGCONFIG_INIT" != 1; then
		PKG_CHECK_MODULES(PKGCONFIGINIT, pkgconfiginit, [],
				  [AC_MSG_RESULT([pkg-config initialized])])
		PKGCONFIG_INIT=1
	fi
}

################################################################################
AC_MSG_CHECKING(for default run directory)
RUN_DIR="/run"
test -d "/run" || RUN_DIR="/var/run"
AC_MSG_RESULT($RUN_DIR)
dnl -- Set up pidfile and run directory
AH_TEMPLATE(DEFAULT_PID_DIR)
AC_ARG_WITH(default-pid-dir,
	    AC_HELP_STRING([--with-default-pid-dir=PID_DIR],
			   [Default directory to keep PID files in. [autodetect]]),
	    DEFAULT_PID_DIR="$withval", DEFAULT_PID_DIR=$RUN_DIR)
AC_DEFINE_UNQUOTED(DEFAULT_PID_DIR, ["$DEFAULT_PID_DIR"],
		   [Default directory to keep PID files in.])

AH_TEMPLATE(DEFAULT_DM_RUN_DIR, [Name of default DM run directory.])
AC_ARG_WITH(default-dm-run-dir,
	    AC_HELP_STRING([--with-default-dm-run-dir=DM_RUN_DIR],
			   [ Default DM run directory. [autodetect]]),
	    DEFAULT_DM_RUN_DIR="$withval", DEFAULT_DM_RUN_DIR=$RUN_DIR)
AC_DEFINE_UNQUOTED(DEFAULT_DM_RUN_DIR, ["$DEFAULT_DM_RUN_DIR"],
		   [Default DM run directory.])

AH_TEMPLATE(DEFAULT_RUN_DIR, [Name of default LVM run directory.])
AC_ARG_WITH(default-run-dir,
	    AC_HELP_STRING([--with-default-run-dir=RUN_DIR],
			   [Default LVM run directory. [autodetect_run_dir/lvm]]),
	    DEFAULT_RUN_DIR="$withval", DEFAULT_RUN_DIR="$RUN_DIR/lvm")
AC_DEFINE_UNQUOTED(DEFAULT_RUN_DIR, ["$DEFAULT_RUN_DIR"],
		   [Default LVM run directory.])

################################################################################
dnl -- Build cluster mirror log daemon
AC_MSG_CHECKING(whether to build cluster mirror log daemon)
AC_ARG_ENABLE(cmirrord,
	      AC_HELP_STRING([--enable-cmirrord],
			     [enable the cluster mirror log daemon]),
	      CMIRRORD=$enableval, CMIRRORD=no)
AC_MSG_RESULT($CMIRRORD)

BUILD_CMIRRORD=$CMIRRORD

################################################################################
dnl -- cmirrord pidfile
if test "$BUILD_CMIRRORD" = yes; then
	AC_ARG_WITH(cmirrord-pidfile,
		    AC_HELP_STRING([--with-cmirrord-pidfile=PATH],
				   [cmirrord pidfile [PID_DIR/cmirrord.pid]]),
		    CMIRRORD_PIDFILE=$withval,
		    CMIRRORD_PIDFILE="$DEFAULT_PID_DIR/cmirrord.pid")
	AC_DEFINE_UNQUOTED(CMIRRORD_PIDFILE, ["$CMIRRORD_PIDFILE"],
			   [Path to cmirrord pidfile.])
fi

################################################################################
dnl -- Look for corosync libraries if required.
if [[ "$BUILD_CMIRRORD" = yes ]]; then
	pkg_config_init

	if test "$HAVE_CPG" != yes; then
		PKG_CHECK_MODULES(CPG, libcpg)
	fi
fi

################################################################################
dnl -- Enable debugging
AC_MSG_CHECKING(whether to enable debugging)
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [enable debugging]),
	      DEBUG=$enableval, DEBUG=no)
AC_MSG_RESULT($DEBUG)

dnl -- Normally turn off optimisation for debug builds
if test "$DEBUG" = yes; then
	COPTIMISE_FLAG=
else
	CSCOPE_CMD=
fi

dnl -- Check if compiler supports -Wjump-misses-init
AC_TRY_CCFLAG([-Wjump-misses-init], [HAVE_WJUMP], [], [])
AC_SUBST(HAVE_WJUMP)
AC_TRY_CCFLAG([-Wclobbered], [HAVE_WCLOBBERED], [], [])
AC_SUBST(HAVE_WCLOBBERED)
AC_TRY_CCFLAG([-Wsync-nand], [HAVE_WSYNCNAND], [], [])
AC_SUBST(HAVE_WSYNCNAND)

################################################################################
dnl -- Override optimisation
AC_MSG_CHECKING(for C optimisation flag)
AC_ARG_WITH(optimisation,
	    AC_HELP_STRING([--with-optimisation=OPT],
			   [C optimisation flag [OPT=-O2]]),
	    COPTIMISE_FLAG=$withval)
AC_MSG_RESULT($COPTIMISE_FLAG)

################################################################################
dnl -- Enable profiling
AC_MSG_CHECKING(whether to gather gcov profiling data)
AC_ARG_ENABLE(profiling,
	      AC_HELP_STRING([--enable-profiling],
			     [gather gcov profiling data]),
	      PROFILING=$enableval, PROFILING=no)
AC_MSG_RESULT($PROFILING)

if test "$PROFILING" = yes; then
  COPTIMISE_FLAG="$COPTIMISE_FLAG -fprofile-arcs -ftest-coverage"
  AC_PATH_TOOL(LCOV, lcov)
  AC_PATH_TOOL(GENHTML, genhtml)
  test -z "$LCOV" -o -z "$GENHTML" && AC_MSG_ERROR([lcov and genhtml are required for profiling])
  AC_PATH_TOOL(GENPNG, genpng)
  if test -n "$GENPNG"; then
    AC_MSG_CHECKING([whether $GENPNG has all required modules])
    if "$GENPNG" --help > /dev/null 2>&1 ; then
      AC_MSG_RESULT(ok)
      GENHTML="$GENHTML --frames"
    else
      AC_MSG_RESULT([not supported])
      AC_MSG_WARN([GD.pm perl module is not installed])
      GENPNG=
    fi
  fi
fi

################################################################################
dnl -- Set LVM2 testsuite data
TESTSUITE_DATA='${datarootdir}/lvm2-testsuite'
# double eval needed ${datarootdir} -> ${prefix}/share -> real path
AC_DEFINE_UNQUOTED(TESTSUITE_DATA, ["$(eval echo $(eval echo $TESTSUITE_DATA))"], [Path to testsuite data])


################################################################################
dnl -- Enable valgrind awareness of memory pools
AC_MSG_CHECKING(whether to enable valgrind awareness of pools)
AC_ARG_ENABLE(valgrind_pool,
	      AC_HELP_STRING([--enable-valgrind-pool],
			     [enable valgrind awareness of pools]),
	      VALGRIND_POOL=$enableval, VALGRIND_POOL=no)
AC_MSG_RESULT($VALGRIND_POOL)

pkg_config_init
PKG_CHECK_MODULES(VALGRIND, valgrind, [HAVE_VALGRIND=yes], [if test x$VALGRIND_POOL = xyes; then AC_MSG_ERROR(bailing out); fi])
AC_SUBST(VALGRIND_CFLAGS)

if test x$HAVE_VALGRIND = xyes; then
	AC_DEFINE([HAVE_VALGRIND], 1, [valgrind.h found])
fi

if test x$VALGRIND_POOL = xyes; then
	AC_DEFINE([VALGRIND_POOL], 1, [Enable a valgrind aware build of pool])
fi

################################################################################
dnl -- Disable devmapper
AC_MSG_CHECKING(whether to use device-mapper)
AC_ARG_ENABLE(devmapper,
	      AC_HELP_STRING([--disable-devmapper],
			     [disable LVM2 device-mapper interaction]),
	      DEVMAPPER=$enableval)
AC_MSG_RESULT($DEVMAPPER)

if test "$DEVMAPPER" = yes; then
	AC_DEFINE([DEVMAPPER_SUPPORT], 1, [Define to 1 to enable LVM2 device-mapper interaction.])
fi

################################################################################
dnl -- Build lvmpolld
AC_MSG_CHECKING(whether to build lvmpolld)
AC_ARG_ENABLE(lvmpolld,
	      AC_HELP_STRING([--enable-lvmpolld],
			     [enable the LVM Polling Daemon]),
	      LVMPOLLD=$enableval)
test -n "$LVMPOLLD" && BUILD_LVMPOLLD=$LVMPOLLD
AC_MSG_RESULT($BUILD_LVMPOLLD)

################################################################################
BUILD_LVMLOCKD=no

dnl -- Build lvmlockdsanlock
AC_MSG_CHECKING(whether to build lvmlockdsanlock)
AC_ARG_ENABLE(lvmlockd-sanlock,
	      AC_HELP_STRING([--enable-lvmlockd-sanlock],
			     [enable the LVM lock daemon using sanlock]),
	      LOCKDSANLOCK=$enableval)
AC_MSG_RESULT($LOCKDSANLOCK)

BUILD_LOCKDSANLOCK=$LOCKDSANLOCK

dnl -- Look for sanlock libraries
if test "$BUILD_LOCKDSANLOCK" = yes; then
	PKG_CHECK_MODULES(LOCKD_SANLOCK, libsanlock_client >= 3.3.0, [HAVE_LOCKD_SANLOCK=yes], $bailout)
	AC_DEFINE([LOCKDSANLOCK_SUPPORT], 1, [Define to 1 to include code that uses lvmlockd sanlock option.])
	BUILD_LVMLOCKD=yes
fi

################################################################################
dnl -- Build lvmlockddlm
AC_MSG_CHECKING(whether to build lvmlockddlm)
AC_ARG_ENABLE(lvmlockd-dlm,
	      AC_HELP_STRING([--enable-lvmlockd-dlm],
			     [enable the LVM lock daemon using dlm]),
	      LOCKDDLM=$enableval)
AC_MSG_RESULT($LOCKDDLM)

BUILD_LOCKDDLM=$LOCKDDLM

dnl -- Look for dlm libraries
if test "$BUILD_LOCKDDLM" = yes; then
	PKG_CHECK_MODULES(LOCKD_DLM, libdlm, [HAVE_LOCKD_DLM=yes], $bailout)
	AC_DEFINE([LOCKDDLM_SUPPORT], 1, [Define to 1 to include code that uses lvmlockd dlm option.])
	BUILD_LVMLOCKD=yes
fi

################################################################################
dnl -- Build lvmlockd
AC_MSG_CHECKING(whether to build lvmlockd)
AC_MSG_RESULT($BUILD_LVMLOCKD)

if test "$BUILD_LVMLOCKD" = yes; then
	AS_IF([test "$LVMPOLLD" = no], [AC_MSG_ERROR([cannot build lvmlockd with --disable-lvmpolld.])])
	AS_IF([test "$BUILD_LVMPOLLD" = no], [BUILD_LVMPOLLD=yes; AC_MSG_WARN([Enabling lvmpolld - required by lvmlockd.])])
	AC_MSG_CHECKING([defaults for use_lvmlockd])
	AC_ARG_ENABLE(use_lvmlockd,
		      AC_HELP_STRING([--disable-use-lvmlockd],
				     [disable usage of LVM lock daemon]),
		      [case ${enableval} in
		       yes) DEFAULT_USE_LVMLOCKD=1 ;;
		       *) DEFAULT_USE_LVMLOCKD=0 ;;
		       esac], DEFAULT_USE_LVMLOCKD=1)
	AC_MSG_RESULT($DEFAULT_USE_LVMLOCKD)
	AC_DEFINE([LVMLOCKD_SUPPORT], 1, [Define to 1 to include code that uses lvmlockd.])

	AC_ARG_WITH(lvmlockd-pidfile,
		    AC_HELP_STRING([--with-lvmlockd-pidfile=PATH],
				   [lvmlockd pidfile [PID_DIR/lvmlockd.pid]]),
		    LVMLOCKD_PIDFILE=$withval,
		    LVMLOCKD_PIDFILE="$DEFAULT_PID_DIR/lvmlockd.pid")
	AC_DEFINE_UNQUOTED(LVMLOCKD_PIDFILE, ["$LVMLOCKD_PIDFILE"],
			   [Path to lvmlockd pidfile.])
else
	DEFAULT_USE_LVMLOCKD=0
fi
AC_DEFINE_UNQUOTED(DEFAULT_USE_LVMLOCKD, [$DEFAULT_USE_LVMLOCKD],
		   [Use lvmlockd by default.])

################################################################################
dnl -- Check lvmpolld
if test "$BUILD_LVMPOLLD" = yes; then
	AC_MSG_CHECKING([defaults for use_lvmpolld])
	AC_ARG_ENABLE(use_lvmpolld,
		      AC_HELP_STRING([--disable-use-lvmpolld],
				     [disable usage of LVM Poll Daemon]),
		      [case ${enableval} in
		       yes) DEFAULT_USE_LVMPOLLD=1 ;;
		       *) DEFAULT_USE_LVMPOLLD=0 ;;
		       esac], DEFAULT_USE_LVMPOLLD=1)
	AC_MSG_RESULT($DEFAULT_USE_LVMPOLLD)
	AC_DEFINE([LVMPOLLD_SUPPORT], 1, [Define to 1 to include code that uses lvmpolld.])

	AC_ARG_WITH(lvmpolld-pidfile,
		    AC_HELP_STRING([--with-lvmpolld-pidfile=PATH],
				   [lvmpolld pidfile [PID_DIR/lvmpolld.pid]]),
		    LVMPOLLD_PIDFILE=$withval,
		    LVMPOLLD_PIDFILE="$DEFAULT_PID_DIR/lvmpolld.pid")
	AC_DEFINE_UNQUOTED(LVMPOLLD_PIDFILE, ["$LVMPOLLD_PIDFILE"],
			   [Path to lvmpolld pidfile.])
else
	DEFAULT_USE_LVMPOLLD=0
fi
AC_DEFINE_UNQUOTED(DEFAULT_USE_LVMPOLLD, [$DEFAULT_USE_LVMPOLLD],
		   [Use lvmpolld by default.])

################################################################################
dnl -- Check dmfilemapd
AC_MSG_CHECKING(whether to build dmfilemapd)
AC_ARG_ENABLE(dmfilemapd, AC_HELP_STRING([--enable-dmfilemapd],
					 [enable the dmstats filemap daemon]),
	      BUILD_DMFILEMAPD=$enableval, BUILD_DMFILEMAPD=no)
AC_MSG_RESULT($BUILD_DMFILEMAPD)
AC_DEFINE([DMFILEMAPD], $BUILD_DMFILEMAPD, [Define to 1 to enable the device-mapper filemap daemon.])

dnl -- dmfilemapd requires FIEMAP
if test "$BUILD_DMFILEMAPD" = yes; then
   AC_CHECK_HEADER([linux/fiemap.h], , [AC_MSG_ERROR(--enable-dmfilemapd requires fiemap.h)])
fi

################################################################################
dnl -- Build notifydbus
AC_MSG_CHECKING(whether to build notifydbus)
AC_ARG_ENABLE(notify-dbus,
	      AC_HELP_STRING([--enable-notify-dbus],
			     [enable LVM notification using dbus]),
	      NOTIFYDBUS_SUPPORT=$enableval, NOTIFYDBUS_SUPPORT=no)
AC_MSG_RESULT($NOTIFYDBUS_SUPPORT)

if test "$NOTIFYDBUS_SUPPORT" = yes; then
	AC_DEFINE([NOTIFYDBUS_SUPPORT], 1, [Define to 1 to include code that uses dbus notification.])
	SYSTEMD_LIBS="-lsystemd"
fi

################################################################################
dnl -- Look for dbus libraries
if test "$NOTIFYDBUS_SUPPORT" = yes; then
	PKG_CHECK_MODULES(NOTIFY_DBUS, systemd >= 221, [HAVE_NOTIFY_DBUS=yes], $bailout)
fi

################################################################################

dnl -- Enable blkid wiping functionality
AC_ARG_ENABLE(blkid_wiping,
	      AC_HELP_STRING([--disable-blkid_wiping],
			     [disable libblkid detection of signatures when wiping and use native code instead]),
	      BLKID_WIPING=$enableval, BLKID_WIPING=maybe)

DEFAULT_USE_BLKID_WIPING=0
if test "$BLKID_WIPING" != no; then
	pkg_config_init
	PKG_CHECK_MODULES(BLKID, blkid >= 2.24,
			  [ BLKID_WIPING=yes
			    BLKID_PC="blkid"
			    DEFAULT_USE_BLKID_WIPING=1
			    AC_DEFINE([BLKID_WIPING_SUPPORT], 1, [Define to 1 to use libblkid detection of signatures when wiping.])
			  ], [if test "$BLKID_WIPING" = maybe; then
				BLKID_WIPING=no
			   else
			        AC_MSG_ERROR([bailing out... blkid library >= 2.24 is required])
			   fi])
fi
AC_MSG_CHECKING([whether to enable libblkid detection of signatures when wiping])
AC_MSG_RESULT($BLKID_WIPING)
AC_DEFINE_UNQUOTED(DEFAULT_USE_BLKID_WIPING, [$DEFAULT_USE_BLKID_WIPING],
		   [Use blkid wiping by default.])

################################################################################
dnl -- Enable udev-systemd protocol to instantiate a service for background jobs
dnl -- Requires systemd version 205 at least (including support for systemd-run)
AC_ARG_ENABLE(udev-systemd-background-jobs,
	      AC_HELP_STRING([--disable-udev-systemd-background-jobs],
			     [disable udev-systemd protocol to instantiate a service for background job]),
	      UDEV_SYSTEMD_BACKGROUND_JOBS=$enableval,
	      UDEV_SYSTEMD_BACKGROUND_JOBS=maybe)

if test "$UDEV_SYSTEMD_BACKGROUND_JOBS" != no; then
	pkg_config_init
	PKG_CHECK_MODULES(SYSTEMD, systemd >= 205,
			  [UDEV_SYSTEMD_BACKGROUND_JOBS=yes],
			  [if test "$UDEV_SYSTEMD_BACKGROUND_JOBS" = maybe; then
				UDEV_SYSTEMD_BACKGROUND_JOBS=no
			   else
				AC_MSG_ERROR([bailing out... systemd >= 205 is required])
			   fi])
fi

AC_MSG_CHECKING(whether to use udev-systemd protocol for jobs in background)
AC_MSG_RESULT($UDEV_SYSTEMD_BACKGROUND_JOBS)

################################################################################
dnl -- Enable udev synchronisation
AC_MSG_CHECKING(whether to enable synchronisation with udev processing)
AC_ARG_ENABLE(udev_sync,
	      AC_HELP_STRING([--enable-udev_sync],
			     [enable synchronisation with udev processing]),
	      UDEV_SYNC=$enableval, UDEV_SYNC=no)
AC_MSG_RESULT($UDEV_SYNC)

if test "$UDEV_SYNC" = yes; then
	pkg_config_init
	PKG_CHECK_MODULES(UDEV, libudev >= 143, [UDEV_PC="libudev"])
	AC_DEFINE([UDEV_SYNC_SUPPORT], 1, [Define to 1 to enable synchronisation with udev processing.])

	AC_CHECK_LIB(udev, udev_device_get_is_initialized, AC_DEFINE([HAVE_LIBUDEV_UDEV_DEVICE_GET_IS_INITIALIZED], 1,
	  [Define to 1 if udev_device_get_is_initialized is available.]))
	LIBS=$ac_check_lib_save_LIBS
fi

dnl -- Enable udev rules
AC_MSG_CHECKING(whether to enable installation of udev rules required for synchronisation)
AC_ARG_ENABLE(udev_rules,
	      AC_HELP_STRING([--enable-udev_rules],
			     [install rule files needed for udev synchronisation]),
	      UDEV_RULES=$enableval, UDEV_RULES=$UDEV_SYNC)
AC_MSG_RESULT($UDEV_RULES)

AC_MSG_CHECKING(whether to enable executable path detection in udev rules)
AC_ARG_ENABLE(udev_rule_exec_detection,
	      AC_HELP_STRING([--enable-udev-rule-exec-detection],
			     [enable executable path detection in udev rules]),
	      UDEV_RULE_EXEC_DETECTION=$enableval, UDEV_RULE_EXEC_DETECTION=no)
AC_MSG_RESULT($UDEV_RULE_EXEC_DETECTION)

dnl -- Check support for built-in blkid against target udev version
if test "$UDEV_RULE" != no ; then
	AC_MSG_CHECKING(whether udev supports built-in blkid)
	pkg_config_init
	if $("$PKG_CONFIG" --atleast-version=176 libudev); then
		UDEV_HAS_BUILTIN_BLKID=yes
	else
		UDEV_HAS_BUILTIN_BLKID=no
	fi
	AC_MSG_RESULT($UDEV_HAS_BUILTIN_BLKID)
fi

################################################################################
dnl -- Compatibility mode
AC_ARG_ENABLE(compat,
	      AC_HELP_STRING([--enable-compat],
			     [enable support for old device-mapper versions]),
	      DM_COMPAT=$enableval, DM_COMPAT=no)

AS_IF([test "$DM_COMPAT" = yes],
      [AC_DEFINE([DM_COMPAT], 1, [Define to enable compat protocol])
      AC_MSG_ERROR([--enable-compat is not currently supported.
Since device-mapper version 1.02.66, only one version (4) of the device-mapper
ioctl protocol is supported.])])

################################################################################
dnl -- Compatible units suffix mode
AC_ARG_ENABLE(units-compat,
	      AC_HELP_STRING([--enable-units-compat],
			     [enable output compatibility with old versions that
			      that do not use KiB-style unit suffixes]),
	      UNITS_COMPAT=$enableval, UNITS_COMPAT=no)

if test "$UNITS_COMPAT" = yes; then
	AC_DEFINE([DEFAULT_SI_UNIT_CONSISTENCY], 0, [Define to 0 to reinstate the pre-2.02.54 handling of unit suffixes.])
fi

################################################################################
dnl -- Disable ioctl
AC_ARG_ENABLE(ioctl,
	      AC_HELP_STRING([--disable-ioctl],
			     [disable ioctl calls to device-mapper in the kernel]),
	      DM_IOCTLS=$enableval)
AS_IF([test "$DM_IOCTLS" = yes],
      [AC_DEFINE([DM_IOCTLS], 1, [Define to enable ioctls calls to kernel])])

################################################################################
dnl -- Disable O_DIRECT
AC_MSG_CHECKING(whether to enable O_DIRECT)
AC_ARG_ENABLE(o_direct,
	      AC_HELP_STRING([--disable-o_direct], [disable O_DIRECT]),
	      ODIRECT=$enableval)
AC_MSG_RESULT($ODIRECT)

if test "$ODIRECT" = yes; then
	AC_DEFINE([O_DIRECT_SUPPORT], 1, [Define to 1 to enable O_DIRECT support.])
fi

################################################################################
dnl -- Enable cmdlib
AC_MSG_CHECKING(whether to compile liblvm2cmd.so)
AC_ARG_ENABLE(cmdlib,
	      AC_HELP_STRING([--enable-cmdlib], [build shared command library]),
	      CMDLIB=$enableval, CMDLIB=no)
AC_MSG_RESULT($CMDLIB)
AC_SUBST([LVM2CMD_LIB])
test "$CMDLIB" = yes \
  && LVM2CMD_LIB=-llvm2cmd \
  || LVM2CMD_LIB=

################################################################################
dnl -- Enable D-Bus service
AC_MSG_CHECKING(whether to include Python D-Bus support)
AC_ARG_ENABLE(dbus-service,
	      AC_HELP_STRING([--enable-dbus-service], [install D-Bus support]),
	      BUILD_LVMDBUSD=$enableval, BUILD_LVMDBUSD=no)
AC_MSG_RESULT($BUILD_LVMDBUSD)
AS_IF([test "$NOTIFYDBUS_SUPPORT" = yes && test "BUILD_LVMDBUSD" = yes],
      [AC_MSG_WARN([Building D-Bus support without D-Bus notifications.])])

################################################################################
dnl -- Enable Python dbus library

if test "$BUILD_LVMDBUSD" = yes; then
	unset am_cv_pathless_PYTHON ac_cv_path_PYTHON am_cv_python_platform
	unset am_cv_python_pythondir am_cv_python_version am_cv_python_pyexecdir
	unset ac_cv_path_PYTHON_CONFIG ac_cv_path_ac_pt_PYTHON_CONFIG
	AM_PATH_PYTHON([3])
	PYTHON3=$PYTHON
	test -z "$PYTHON3" && AC_MSG_ERROR([python3 is required for --enable-python3_bindings or --enable-dbus-service but cannot be found])
	AC_PATH_TOOL(PYTHON3_CONFIG, python3-config)
	test -z "$PYTHON3_CONFIG" && AC_MSG_ERROR([python3 headers are required for --enable-python3_bindings or --enable-dbus-service but cannot be found])
	PYTHON3_INCDIRS=`"$PYTHON3_CONFIG" --includes`
	PYTHON3_LIBDIRS=`"$PYTHON3_CONFIG" --libs`
	PYTHON3DIR=$pythondir
	test "$PYTHON3_BINDINGS" = yes && PYTHON_BINDINGS=yes

	# To get this macro, install autoconf-archive package then run autoreconf
	AC_PYTHON_MODULE([pyudev], [Required], python3)
	AC_PYTHON_MODULE([dbus], [Required], python3)
fi

################################################################################
dnl -- Enable pkg-config
AC_ARG_ENABLE(pkgconfig,
	      AC_HELP_STRING([--enable-pkgconfig], [install pkgconfig support]),
	      PKGCONFIG=$enableval, PKGCONFIG=no)

################################################################################
dnl -- Enable installation of writable files by user
AC_ARG_ENABLE(write_install,
	      AC_HELP_STRING([--enable-write_install],
			     [install user writable files]),
	      WRITE_INSTALL=$enableval, WRITE_INSTALL=no)

################################################################################
dnl -- Enable fsadm
AC_MSG_CHECKING(whether to install fsadm)
AC_ARG_ENABLE(fsadm, AC_HELP_STRING([--disable-fsadm], [disable fsadm]),
	      FSADM=$enableval)
AC_MSG_RESULT($FSADM)

################################################################################
dnl -- Enable blkdeactivate
AC_MSG_CHECKING(whether to install blkdeactivate)
AC_ARG_ENABLE(blkdeactivate, AC_HELP_STRING([--disable-blkdeactivate], [disable blkdeactivate]),
	      BLKDEACTIVATE=$enableval)
AC_MSG_RESULT($BLKDEACTIVATE)

################################################################################
dnl -- enable dmeventd handling
AC_MSG_CHECKING(whether to use dmeventd)
AC_ARG_ENABLE(dmeventd, AC_HELP_STRING([--enable-dmeventd],
				       [enable the device-mapper event daemon]),
	      BUILD_DMEVENTD=$enableval, BUILD_DMEVENTD=no)
AC_MSG_RESULT($BUILD_DMEVENTD)

dnl -- dmeventd currently requires internal mirror support
if test "$BUILD_DMEVENTD" = yes; then
   if test "$MIRRORS" != internal; then
      AC_MSG_ERROR([--enable-dmeventd currently requires --with-mirrors=internal])
   fi
   if test "$CMDLIB" = no; then
      AC_MSG_ERROR([--enable-dmeventd requires --enable-cmdlib to be used as well])
   fi

   AC_DEFINE([DMEVENTD], 1, [Define to 1 to enable the device-mapper event daemon.])
fi

################################################################################
dnl -- getline included in recent libc

AC_CHECK_LIB(c, getline, AC_DEFINE([HAVE_GETLINE], 1,
  [Define to 1 if getline is available.]))

################################################################################
dnl -- canonicalize_file_name included in recent libc

AC_CHECK_LIB(c, canonicalize_file_name,
  AC_DEFINE([HAVE_CANONICALIZE_FILE_NAME], 1,
    [Define to 1 if canonicalize_file_name is available.]))

################################################################################
dnl -- Check for dlopen
AC_CHECK_LIB(dl, dlopen,
	     [AC_DEFINE([HAVE_LIBDL], 1, [Define to 1 if dynamic libraries are available.])
	      DL_LIBS="-ldl"
	      HAVE_LIBDL=yes],
	     [DL_LIBS=
	      HAVE_LIBDL=no ])

################################################################################
dnl -- Check for shared/static conflicts
if [[ \( "$LVM1" = shared -o "$POOL" = shared \
      \) -a "$STATIC_LINK" = yes ]]; then
	AC_MSG_ERROR([Features cannot be 'shared' when building statically])
fi

################################################################################
AC_CHECK_LIB(m, log10,
	[M_LIBS="-lm"], hard_bailout)

################################################################################
AC_CHECK_LIB([pthread], [pthread_mutex_lock],
	[PTHREAD_LIBS="-lpthread"], hard_bailout)

################################################################################
dnl -- Disable selinux
AC_MSG_CHECKING(whether to enable selinux support)
AC_ARG_ENABLE(selinux,
	      AC_HELP_STRING([--disable-selinux], [disable selinux support]),
	      SELINUX=$enableval)
AC_MSG_RESULT($SELINUX)

################################################################################
dnl -- Check for selinux
if test "$SELINUX" = yes; then
	AC_CHECK_LIB([sepol], [sepol_check_context], [
		AC_DEFINE([HAVE_SEPOL], 1, [Define to 1 if sepol_check_context is available.])
		SELINUX_LIBS="-lsepol"])

	AC_CHECK_LIB([selinux], [is_selinux_enabled], [
		AC_CHECK_HEADERS([selinux/selinux.h],, hard_bailout)
		AC_CHECK_HEADERS([selinux/label.h])
		AC_DEFINE([HAVE_SELINUX], 1, [Define to 1 to include support for selinux.])
		SELINUX_LIBS="-lselinux $SELINUX_LIBS"
		SELINUX_PC="libselinux"
		HAVE_SELINUX=yes ], [
		AC_MSG_WARN(Disabling selinux)
		SELINUX_LIBS=
		SELINUX_PC=
		HAVE_SELINUX=no ])
fi

################################################################################
dnl -- Check for realtime clock support
RT_LIBS=
HAVE_REALTIME=no
if test "$REALTIME" = yes; then
	AC_CHECK_FUNCS([clock_gettime], HAVE_REALTIME=yes)

	AS_IF([test "$HAVE_REALTIME" != yes], [ # try again with -lrt
	      AC_CHECK_LIB([rt], [clock_gettime], RT_LIBS="-lrt"; HAVE_REALTIME=yes)])

	if test "$HAVE_REALTIME" = yes; then
		AC_DEFINE([HAVE_REALTIME], 1, [Define to 1 to include support for realtime clock.])
	else
		AC_MSG_WARN(Disabling realtime clock)
	fi
fi

dnl Check if the system has struct stat st_ctim.
AC_CACHE_CHECK([for struct stat has st_ctim.],
	[ac_cv_stat_st_ctim],
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[#include <sys/stat.h>
long bar(void) { struct stat s; return (long)(s.st_ctim.tv_sec + s.st_ctim.tv_nsec);}]
	 )], [ac_cv_stat_st_ctim=yes], [ac_cv_stat_st_ctim=no])])

AC_IF_YES(ac_cv_stat_st_ctim,
	  AC_DEFINE(HAVE_STAT_ST_CTIM, 1,
		    [Define if struct stat has a field st_ctim with timespec for ctime]))

################################################################################
dnl -- Check for getopt
AC_CHECK_HEADERS(getopt.h, AC_DEFINE([HAVE_GETOPTLONG], 1, [Define to 1 if getopt_long is available.]))

################################################################################
dnl -- Check for readline (Shamelessly copied from parted 1.4.17)
if test "$READLINE" != no; then
	lvm_saved_libs=$LIBS
	AC_SEARCH_LIBS([tgetent], [tinfo ncurses curses termcap termlib],
		READLINE_LIBS=$ac_cv_search_tgetent, [
		if test "$READLINE" = yes; then
			AC_MSG_ERROR(
[termcap could not be found which is required for the
--enable-readline option (which is enabled by default).  Either disable readline
support with --disable-readline or download and install termcap from:
	ftp.gnu.org/gnu/termcap
Note: if you are using precompiled packages you will also need the development
  package as well (which may be called termcap-devel or something similar).
Note: (n)curses also seems to work as a substitute for termcap.  This was
  not found either - but you could try installing that as well.])
		fi])
	dnl -- Old systems may need extra termcap dependency explicitly in LIBS
	AC_CHECK_LIB([readline], [readline], [
		AC_DEFINE([READLINE_SUPPORT], 1,
			[Define to 1 to include the LVM readline shell.])
		dnl -- Try only with -lreadline and check for different symbol
		READLINE=yes
		LIBS=$lvm_saved_libs
		AC_CHECK_LIB([readline], [rl_line_buffer],
			[ READLINE_LIBS="-lreadline" ], [
			  AC_MSG_RESULT([linking -lreadline with $READLINE_LIBS needed])
			  READLINE_LIBS="-lreadline $READLINE_LIBS"
			]) ], [
		READLINE_LIBS=
		if test "$READLINE" = yes; then
			AC_MSG_ERROR(
[GNU Readline could not be found which is required for the
--enable-readline option (which is enabled by default).  Either disable readline
support with --disable-readline or download and install readline from:
	ftp.gnu.org/gnu/readline
Note: if you are using precompiled packages you will also need the development
package as well (which may be called readline-devel or something similar).])
		fi ])
	LIBS="$READLINE_LIBS $lvm_saved_libs"
	AC_CHECK_FUNCS([rl_completion_matches])
	LIBS=$lvm_saved_libs
fi

################################################################################
dnl -- Internationalisation stuff
AC_MSG_CHECKING(whether to enable internationalisation)
AC_ARG_ENABLE(nls,
	      AC_HELP_STRING([--enable-nls], [enable Native Language Support]),
	      INTL=$enableval, INTL=no)
AC_MSG_RESULT($INTL)

if test "$INTL" = yes; then
# FIXME - Move this - can be device-mapper too
	INTL_PACKAGE="lvm2"
	AC_PATH_TOOL(MSGFMT, msgfmt)

	AS_IF([test -z "$MSGFMT"], [AC_MSG_ERROR([msgfmt not found in path $PATH])])

	AC_ARG_WITH(localedir,
		    AC_HELP_STRING([--with-localedir=DIR],
				   [locale-dependent data [DATAROOTDIR/locale]]),
		    localedir=$withval, localedir=${localedir-'${datarootdir}/locale'})
	AC_DEFINE_UNQUOTED([INTL_PACKAGE], ["$INTL_PACKAGE"], [Internalization package])
	# double eval needed ${datarootdir} -> ${prefix}/share -> real path
	AC_DEFINE_UNQUOTED([LOCALEDIR], ["$(eval echo $(eval echo $localedir))"], [Locale-dependent data])
fi

################################################################################
dnl -- FIXME: need to switch to regular option here  --sysconfdir
AC_ARG_WITH(confdir,
	    AC_HELP_STRING([--with-confdir=DIR],
			   [configuration files in DIR [/etc]]),
	    CONFDIR=$withval, CONFDIR='/etc')
AC_DEFINE_UNQUOTED(DEFAULT_ETC_DIR, ["$CONFDIR"],
		   [Default system configuration directory.])

AC_ARG_WITH(staticdir,
	    AC_HELP_STRING([--with-staticdir=DIR],
			   [static binaries in DIR [EPREFIX/sbin]]),
	    STATICDIR=$withval, STATICDIR='${exec_prefix}/sbin')

AC_ARG_WITH(usrlibdir,
	    AC_HELP_STRING([--with-usrlibdir=DIR],
			   [usrlib in DIR [PREFIX/lib]]),
	    usrlibdir=$withval, usrlibdir='${prefix}/lib')

AC_ARG_WITH(usrsbindir,
	    AC_HELP_STRING([--with-usrsbindir=DIR],
			   [usrsbin executables in DIR [PREFIX/sbin]]),
	    usrsbindir=$withval, usrsbindir='${prefix}/sbin')

################################################################################
AC_ARG_WITH(udev_prefix,
	    AC_HELP_STRING([--with-udev-prefix=UPREFIX],
			   [install udev rule files in UPREFIX [EPREFIX]]),
	    udev_prefix=$withval, udev_prefix='${exec_prefix}')

AC_ARG_WITH(udevdir,
	    AC_HELP_STRING([--with-udevdir=DIR],
			   [udev rules in DIR [UPREFIX/lib/udev/rules.d]]),
	    udevdir=$withval, udevdir='${udev_prefix}/lib/udev/rules.d')

################################################################################
dnl -- Get the systemd system unit dir value from pkg_config automatically if value not given explicitly.
dnl -- This follows the recommendation for systemd integration best practices mentioned in daemon(7) manpage.
AC_ARG_WITH(systemdsystemunitdir,
	    AC_HELP_STRING([--with-systemdsystemunitdir=DIR],
			   [systemd service files in DIR]),
	    systemdsystemunitdir=$withval,
	    pkg_config_init
	    pkg_systemdsystemunitdir=$("$PKG_CONFIG" --variable=systemdsystemunitdir systemd))

test -n "$pkg_systemdsystemunitdir" && systemdsystemunitdir=$pkg_systemdsystemunitdir
test -z "$systemdsystemunitdir" && systemdsystemunitdir='${exec_prefix}/lib/systemd/system';

systemdutildir=$("$PKG_CONFIG" --variable=systemdutildir systemd)
test -z "$systemdutildir" && systemdutildir='${exec_prefix}/lib/systemd';

################################################################################
AC_ARG_WITH(tmpfilesdir,
	    AC_HELP_STRING([--with-tmpfilesdir=DIR],
			   [install configuration files for management of volatile files and directories in DIR [PREFIX/lib/tmpfiles.d]]),
			   tmpfilesdir=$withval, tmpfilesdir='${prefix}/lib/tmpfiles.d')
################################################################################
dnl -- Ensure additional headers required
if test "$READLINE" = yes; then
	AC_CHECK_HEADERS(readline/readline.h readline/history.h,,hard_bailout)
fi
AC_MSG_CHECKING(whether to enable readline)
AC_MSG_RESULT($READLINE)

if test "$BUILD_CMIRRORD" = yes; then
	AC_CHECK_FUNCS(atexit,,hard_bailout)
fi

if test "$BUILD_LVMLOCKD" = yes; then
	AS_IF([test "$HAVE_REALTIME" != yes], [AC_MSG_ERROR([Realtime clock support is mandatory for lvmlockd.])])
	AC_CHECK_FUNCS(strtoull,,hard_bailout)
fi

if test "$BUILD_LVMPOLLD" = yes; then
	AC_CHECK_FUNCS(strpbrk,,hard_bailout)
	AC_FUNC_STRERROR_R
fi

if test "$BUILD_DMEVENTD" = yes; then
	AC_CHECK_HEADERS(arpa/inet.h,,hard_bailout)
fi

if test "$HAVE_LIBDL" = yes; then
	AC_CHECK_HEADERS(dlfcn.h,,hard_bailout)
fi

if test "$INTL" = yes; then
	AC_CHECK_HEADERS(libintl.h,,hard_bailout)
fi

if test "$UDEV_SYNC" = yes; then
	AC_CHECK_HEADERS(sys/ipc.h sys/sem.h,,hard_bailout)
fi

if test "$BUILD_DMFILEMAPD" = yes; then
	AC_CHECK_HEADERS([sys/inotify.h],,hard_bailout)
fi

################################################################################
AC_PATH_TOOL(MODPROBE_CMD, modprobe, [], [$PATH_SBIN])

if test -n "$MODPROBE_CMD"; then
	AC_DEFINE_UNQUOTED([MODPROBE_CMD], ["$MODPROBE_CMD"], [The path to 'modprobe', if available.])
fi

SYSCONFDIR="$(eval echo $(eval echo $sysconfdir))"

SBINDIR="$(eval echo $(eval echo $sbindir))"
LVM_PATH="$SBINDIR/lvm"
AC_DEFINE_UNQUOTED(LVM_PATH, ["$LVM_PATH"], [Path to lvm binary.])

LVMCONFIG_PATH="$SBINDIR/lvmconfig"
AC_DEFINE_UNQUOTED(LVMCONFIG_PATH, ["$LVMCONFIG_PATH"], [Path to lvmconfig binary.])

USRSBINDIR="$(eval echo $(eval echo $usrsbindir))"

FSADM_PATH="$SBINDIR/fsadm"
AC_DEFINE_UNQUOTED(FSADM_PATH, ["$FSADM_PATH"], [Path to fsadm binary.])

################################################################################
dnl -- dmeventd pidfile and executable path
if test "$BUILD_DMEVENTD" = yes; then
	AC_ARG_WITH(dmeventd-pidfile,
		    AC_HELP_STRING([--with-dmeventd-pidfile=PATH],
				   [dmeventd pidfile [PID_DIR/dmeventd.pid]]),
		    DMEVENTD_PIDFILE=$withval,
		    DMEVENTD_PIDFILE="$DEFAULT_PID_DIR/dmeventd.pid")
	AC_DEFINE_UNQUOTED(DMEVENTD_PIDFILE, ["$DMEVENTD_PIDFILE"],
			   [Path to dmeventd pidfile.])
fi

if test "$BUILD_DMEVENTD" = yes; then
	AC_ARG_WITH(dmeventd-path,
		    AC_HELP_STRING([--with-dmeventd-path=PATH],
				   [dmeventd path [EPREFIX/sbin/dmeventd]]),
		    DMEVENTD_PATH=$withval,
		    DMEVENTD_PATH="$SBINDIR/dmeventd")
	AC_DEFINE_UNQUOTED(DMEVENTD_PATH, ["$DMEVENTD_PATH"],
			   [Path to dmeventd binary.])
fi

################################################################################
dnl -- various defaults
dnl -- FIXME: need to switch to regular option here  --sysconfdir
AC_ARG_WITH(default-system-dir,
	    AC_HELP_STRING([--with-default-system-dir=DIR],
			   [default LVM system directory [/etc/lvm]]),
	    DEFAULT_SYS_DIR=$withval, DEFAULT_SYS_DIR="/etc/lvm")
AC_DEFINE_UNQUOTED(DEFAULT_SYS_DIR, ["$DEFAULT_SYS_DIR"],
		   [Path to LVM system directory.])

AC_ARG_WITH(default-profile-subdir,
	    AC_HELP_STRING([--with-default-profile-subdir=SUBDIR],
			   [default configuration profile subdir [profile]]),
	    DEFAULT_PROFILE_SUBDIR=$withval, DEFAULT_PROFILE_SUBDIR=profile)
AC_DEFINE_UNQUOTED(DEFAULT_PROFILE_SUBDIR, ["$DEFAULT_PROFILE_SUBDIR"],
		   [Name of default configuration profile subdirectory.])

AC_ARG_WITH(default-archive-subdir,
	    AC_HELP_STRING([--with-default-archive-subdir=SUBDIR],
			   [default metadata archive subdir [archive]]),
	    DEFAULT_ARCHIVE_SUBDIR=$withval, DEFAULT_ARCHIVE_SUBDIR=archive)
AC_DEFINE_UNQUOTED(DEFAULT_ARCHIVE_SUBDIR, ["$DEFAULT_ARCHIVE_SUBDIR"],
		   [Name of default metadata archive subdirectory.])

AC_ARG_WITH(default-backup-subdir,
	    AC_HELP_STRING([--with-default-backup-subdir=SUBDIR],
			   [default metadata backup subdir [backup]]),
	    DEFAULT_BACKUP_SUBDIR=$withval, DEFAULT_BACKUP_SUBDIR=backup)
AC_DEFINE_UNQUOTED(DEFAULT_BACKUP_SUBDIR, ["$DEFAULT_BACKUP_SUBDIR"],
                   [Name of default metadata backup subdirectory.])

AC_ARG_WITH(default-cache-subdir,
	    AC_HELP_STRING([--with-default-cache-subdir=SUBDIR],
			   [default metadata cache subdir [cache]]),
	    DEFAULT_CACHE_SUBDIR=$withval, DEFAULT_CACHE_SUBDIR=cache)
AC_DEFINE_UNQUOTED(DEFAULT_CACHE_SUBDIR, ["$DEFAULT_CACHE_SUBDIR"],
		   [Name of default metadata cache subdirectory.])

# Select default system locking dir, prefer /run/lock over /var/lock
DEFAULT_SYS_LOCK_DIR="$RUN_DIR/lock"
test -d "$DEFAULT_SYS_LOCK_DIR" || DEFAULT_SYS_LOCK_DIR="/var/lock"

# Support configurable locking subdir for lvm
AC_ARG_WITH(default-locking-dir,
	    AC_HELP_STRING([--with-default-locking-dir=DIR],
			   [default locking directory [autodetect_lock_dir/lvm]]),
	    DEFAULT_LOCK_DIR=$withval,
	    [AC_MSG_CHECKING(for default lock directory)
	     DEFAULT_LOCK_DIR="$DEFAULT_SYS_LOCK_DIR/lvm"
	     AC_MSG_RESULT($DEFAULT_LOCK_DIR)])
AC_DEFINE_UNQUOTED(DEFAULT_LOCK_DIR, ["$DEFAULT_LOCK_DIR"],
		   [Name of default locking directory.])

################################################################################
dnl -- Setup default data alignment
AC_ARG_WITH(default-data-alignment,
	    AC_HELP_STRING([--with-default-data-alignment=NUM],
			   [set the default data alignment in MiB [1]]),
	    DEFAULT_DATA_ALIGNMENT=$withval, DEFAULT_DATA_ALIGNMENT=1)
AC_DEFINE_UNQUOTED(DEFAULT_DATA_ALIGNMENT, [$DEFAULT_DATA_ALIGNMENT],
		   [Default data alignment.])

################################################################################
dnl -- which kernel interface to use (ioctl only)
AC_MSG_CHECKING(for kernel interface choice)
AC_ARG_WITH(interface,
	    AC_HELP_STRING([--with-interface=IFACE],
			   [choose kernel interface (ioctl) [ioctl]]),
	    interface=$withval, interface=ioctl)
test "$interface" != ioctl && AC_MSG_ERROR([--with-interface=ioctl required. fs no longer supported.])
AC_MSG_RESULT($interface)

################################################################################
read DM_LIB_VERSION < "$srcdir"/VERSION_DM 2>/dev/null || DM_LIB_VERSION=Unknown
AC_DEFINE_UNQUOTED(DM_LIB_VERSION, "$DM_LIB_VERSION", [Library version])

DM_LIB_PATCHLEVEL=`cat "$srcdir"/VERSION_DM | $AWK -F '[[-. ]]' '{printf "%s.%s.%s",$1,$2,$3}'`

read VER < "$srcdir"/VERSION 2>/dev/null || VER=Unknown

LVM_VERSION=\"$VER\"
LVM_RELEASE_DATE="\"`echo $VER | $SED 's/.* (//;s/).*//'`\""
VER=`echo "$VER" | $AWK '{print $1}'`
LVM_RELEASE="\"`echo "$VER" | $AWK -F '-' '{print $2}'`\""
VER=`echo "$VER" | $AWK -F '-' '{print $1}'`
LVM_MAJOR=`echo "$VER" | $AWK -F '.' '{print $1}'`
LVM_MINOR=`echo "$VER" | $AWK -F '.' '{print $2}'`
LVM_PATCHLEVEL=`echo "$VER" | $AWK -F '[[(.]]' '{print $3}'`
LVM_LIBAPI=`echo "$VER" | $AWK -F '[[()]]' '{print $2}'`

AC_DEFINE_UNQUOTED(LVM_CONFIGURE_LINE, "$CONFIGURE_LINE", [configure command line used])

################################################################################
AC_SUBST(AWK)
AC_SUBST(BLKID_PC)
AC_SUBST(BUILD_CMIRRORD)
AC_SUBST(BUILD_DMEVENTD)
AC_SUBST(BUILD_LVMDBUSD)
AC_SUBST(BUILD_LVMPOLLD)
AC_SUBST(BUILD_LVMLOCKD)
AC_SUBST(BUILD_LOCKDSANLOCK)
AC_SUBST(BUILD_LOCKDDLM)
AC_SUBST(BUILD_DMFILEMAPD)
AC_SUBST(CACHE)
AC_SUBST(CFLAGS)
AC_SUBST(CFLOW_CMD)
AC_SUBST(CHMOD)
AC_SUBST(CLDFLAGS)
AC_SUBST(CLDNOWHOLEARCHIVE)
AC_SUBST(CLDWHOLEARCHIVE)
AC_SUBST(CMDLIB)
AC_SUBST(CONFDB_CFLAGS)
AC_SUBST(CONFDB_LIBS)
AC_SUBST(CONFDIR)
AC_SUBST(COPTIMISE_FLAG)
AC_SUBST(CPG_CFLAGS)
AC_SUBST(CPG_LIBS)
AC_SUBST(CSCOPE_CMD)
AC_SUBST(DEBUG)
AC_SUBST(DEFAULT_ARCHIVE_SUBDIR)
AC_SUBST(DEFAULT_BACKUP_SUBDIR)
AC_SUBST(DEFAULT_CACHE_SUBDIR)
AC_SUBST(DEFAULT_DATA_ALIGNMENT)
AC_SUBST(DEFAULT_DM_RUN_DIR)
AC_SUBST(DEFAULT_LOCK_DIR)
AC_SUBST(DEFAULT_MIRROR_SEGTYPE)
AC_SUBST(DEFAULT_PID_DIR)
AC_SUBST(DEFAULT_PROFILE_SUBDIR)
AC_SUBST(DEFAULT_RAID10_SEGTYPE)
AC_SUBST(DEFAULT_RUN_DIR)
AC_SUBST(DEFAULT_SPARSE_SEGTYPE)
AC_SUBST(DEFAULT_SYS_DIR)
AC_SUBST(DEFAULT_SYS_LOCK_DIR)
AC_SUBST(DEFAULT_USE_BLKID_WIPING)
AC_SUBST(DEFAULT_USE_LVMPOLLD)
AC_SUBST(DEFAULT_USE_LVMLOCKD)
AC_SUBST(DEVMAPPER)
AC_SUBST(DLM_CFLAGS)
AC_SUBST(DLM_LIBS)
AC_SUBST(DL_LIBS)
AC_SUBST(DMEVENTD_PATH)
AC_SUBST(DM_LIB_PATCHLEVEL)
AC_SUBST(ELDFLAGS)
AC_SUBST(FSADM)
AC_SUBST(FSADM_PATH)
AC_SUBST(BLKDEACTIVATE)
AC_SUBST(HAVE_LIBDL)
AC_SUBST(HAVE_REALTIME)
AC_SUBST(HAVE_VALGRIND)
AC_SUBST(INTL)
AC_SUBST(JOBS)
AC_SUBST(LDDEPS)
AC_SUBST(LIBS)
AC_SUBST(LIB_SUFFIX)
AC_SUBST(LVM_VERSION)
AC_SUBST(LVM_LIBAPI)
AC_SUBST(LVM_MAJOR)
AC_SUBST(LVM_MINOR)
AC_SUBST(LVM_PATCHLEVEL)
AC_SUBST(LVM_PATH)
AC_SUBST(LVM_RELEASE)
AC_SUBST(LVM_RELEASE_DATE)
AC_SUBST(localedir)
AC_SUBST(MANGLING)
AC_SUBST(MIRRORS)
AC_SUBST(MSGFMT)
AC_SUBST(OCF)
AC_SUBST(OCFDIR)
AC_SUBST(ODIRECT)
AC_SUBST(PKGCONFIG)
AC_SUBST(M_LIBS)
AC_SUBST(PTHREAD_LIBS)
AC_SUBST(PYTHON2)
AC_SUBST(PYTHON3)
AC_SUBST(PYTHON_BINDINGS)
AC_SUBST(PYTHON2_BINDINGS)
AC_SUBST(PYTHON3_BINDINGS)
AC_SUBST(PYTHON2_INCDIRS)
AC_SUBST(PYTHON3_INCDIRS)
AC_SUBST(PYTHON2_LIBDIRS)
AC_SUBST(PYTHON3_LIBDIRS)
AC_SUBST(PYTHON2DIR)
AC_SUBST(PYTHON3DIR)
AC_SUBST(QUORUM_CFLAGS)
AC_SUBST(QUORUM_LIBS)
AC_SUBST(RT_LIBS)
AC_SUBST(READLINE_LIBS)
AC_SUBST(REPLICATORS)
AC_SUBST(SACKPT_CFLAGS)
AC_SUBST(SACKPT_LIBS)
AC_SUBST(SALCK_CFLAGS)
AC_SUBST(SALCK_LIBS)
AC_SUBST(SBINDIR)
AC_SUBST(SELINUX_LIBS)
AC_SUBST(SELINUX_PC)
AC_SUBST(SYSCONFDIR)
AC_SUBST(SYSTEMD_LIBS)
AC_SUBST(SNAPSHOTS)
AC_SUBST(STATICDIR)
AC_SUBST(STATIC_LINK)
AC_SUBST(TESTSUITE_DATA)
AC_SUBST(THIN)
AC_SUBST(THIN_CHECK_CMD)
AC_SUBST(THIN_DUMP_CMD)
AC_SUBST(THIN_REPAIR_CMD)
AC_SUBST(THIN_RESTORE_CMD)
AC_SUBST(CACHE_CHECK_CMD)
AC_SUBST(CACHE_DUMP_CMD)
AC_SUBST(CACHE_REPAIR_CMD)
AC_SUBST(CACHE_RESTORE_CMD)
AC_SUBST(UDEV_PC)
AC_SUBST(UDEV_RULES)
AC_SUBST(UDEV_SYNC)
AC_SUBST(UDEV_SYSTEMD_BACKGROUND_JOBS)
AC_SUBST(UDEV_RULE_EXEC_DETECTION)
AC_SUBST(UDEV_HAS_BUILTIN_BLKID)
AC_SUBST(USE_TRACKING)
AC_SUBST(SILENT_RULES)
AC_SUBST(USRSBINDIR)
AC_SUBST(VALGRIND_POOL)
AC_SUBST(VDO)
AC_SUBST(VDO_FORMAT_CMD)
AC_SUBST(VDO_INCLUDE)
AC_SUBST(VDO_LIB)
AC_SUBST(WRITE_INSTALL)
AC_SUBST(DMEVENTD_PIDFILE)
AC_SUBST(LVMPOLLD_PIDFILE)
AC_SUBST(LVMLOCKD_PIDFILE)
AC_SUBST(CMIRRORD_PIDFILE)
AC_SUBST(interface)
AC_SUBST(kerneldir)
AC_SUBST(missingkernel)
AC_SUBST(kernelvsn)
AC_SUBST(tmpdir)
AC_SUBST(udev_prefix)
AC_SUBST(udevdir)
AC_SUBST(systemdsystemunitdir)
AC_SUBST(systemdutildir)
AC_SUBST(tmpfilesdir)
AC_SUBST(usrlibdir)
AC_SUBST(usrsbindir)

################################################################################
dnl -- First and last lines should not contain files to generate in order to
dnl -- keep utility scripts running properly
AC_CONFIG_FILES([
Makefile
make.tmpl
libdm/make.tmpl
daemons/Makefile
daemons/cmirrord/Makefile
daemons/dmeventd/Makefile
daemons/dmeventd/libdevmapper-event.pc
daemons/dmeventd/plugins/Makefile
daemons/dmeventd/plugins/lvm2/Makefile
daemons/dmeventd/plugins/raid/Makefile
daemons/dmeventd/plugins/mirror/Makefile
daemons/dmeventd/plugins/snapshot/Makefile
daemons/dmeventd/plugins/thin/Makefile
daemons/dmeventd/plugins/vdo/Makefile
daemons/lvmdbusd/Makefile
daemons/lvmdbusd/lvmdbusd
daemons/lvmdbusd/lvmdb.py
daemons/lvmdbusd/lvm_shell_proxy.py
daemons/lvmdbusd/path.py
daemons/lvmpolld/Makefile
daemons/lvmlockd/Makefile
conf/Makefile
conf/example.conf
conf/lvmlocal.conf
conf/command_profile_template.profile
conf/metadata_profile_template.profile
include/Makefile
lib/Makefile
include/lvm-version.h
libdaemon/Makefile
libdaemon/client/Makefile
libdaemon/server/Makefile
libdm/Makefile
libdm/dm-tools/Makefile
libdm/libdevmapper.pc
man/Makefile
po/Makefile
scripts/lvm2-pvscan.service
scripts/blkdeactivate.sh
scripts/blk_availability_init_red_hat
scripts/blk_availability_systemd_red_hat.service
scripts/cmirrord_init_red_hat
scripts/com.redhat.lvmdbus1.service
scripts/dm_event_systemd_red_hat.service
scripts/dm_event_systemd_red_hat.socket
scripts/lvm2_cmirrord_systemd_red_hat.service
scripts/lvm2_lvmdbusd_systemd_red_hat.service
scripts/lvm2_lvmpolld_init_red_hat
scripts/lvm2_lvmpolld_systemd_red_hat.service
scripts/lvm2_lvmpolld_systemd_red_hat.socket
scripts/lvmlockd.service
scripts/lvmlocks.service
scripts/lvm2_monitoring_init_red_hat
scripts/lvm2_monitoring_systemd_red_hat.service
scripts/lvm2_tmpfiles_red_hat.conf
scripts/lvmdump.sh
scripts/Makefile
test/Makefile
tools/Makefile
udev/Makefile
])
AC_OUTPUT

AS_IF([test -n "$THIN_CONFIGURE_WARN"],
      [AC_MSG_WARN([Support for thin provisioning is limited since some thin provisioning tools are missing!])])

AS_IF([test -n "$THIN_CHECK_VERSION_WARN"],
      [AC_MSG_WARN([You should also install latest thin_check vsn 0.7.0 (or later) for lvm2 thin provisioning])])

AS_IF([test -n "$CACHE_CONFIGURE_WARN"],
      [AC_MSG_WARN([Support for cache is limited since some cache tools are missing!])])

AS_IF([test -n "$CACHE_CHECK_VERSION_WARN"],
      [AC_MSG_WARN([You should install latest cache_check vsn 0.7.0 to use lvm2 cache metadata format 2])])

AS_IF([test -n "$VDO_CONFIGURE_WARN"],
      [AC_MSG_WARN([unrecognized 'vdoformat' tool is REQUIRED for VDO logical volume creation!])])


AS_IF([test "$ODIRECT" != yes],
      [AC_MSG_WARN([O_DIRECT disabled: low-memory pvmove may lock up])])
