AC_PREREQ([2.60])

m4_define([RELEASE], 2.2.0)

AC_INIT([mtd-utils], [RELEASE], [linux-mtd@lists.infradead.org], mtd-utils)

AC_ARG_ENABLE([unit-tests],
	[AS_HELP_STRING([--enable-unit-tests], [Compile unit test programs])],
	[], [enable_unit_tests="no"])

AS_IF([test "x$enable_unit_tests" = "xyes"], [: ${CFLAGS=""}], [])


AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-bzip2])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_INSTALL
# autoconf <= 2.63 doesn't have AC_PROG_AR
AC_CHECK_TOOL(AR, ar, :)
AC_PROG_RANLIB
AC_SYS_LARGEFILE

m4_ifndef([PKG_PROG_PKG_CONFIG],
  [m4_fatal([Could not locate the pkg-config autoconf
    macros. These are usually located in /usr/share/aclocal/pkg.m4.
    If your macros are in a different location, try setting the
    environment variable AL_OPTS="-I/other/macro/dir" before running
    ./autogen.sh or autoreconf again. Make sure pkg-config is installed.])])
PKG_PROG_PKG_CONFIG

## compiler warnings
UL_WARN_ADD([-Wall])
UL_WARN_ADD([-Wextra])
UL_WARN_ADD([-Wunused])
UL_WARN_ADD([-Wmissing-prototypes])
UL_WARN_ADD([-Wmissing-declarations])
UL_WARN_ADD([-Wwrite-strings])
UL_WARN_ADD([-Wjump-misses-init])
UL_WARN_ADD([-Wuninitialized])
UL_WARN_ADD([-Winit-self])
UL_WARN_ADD([-Wlogical-op])
UL_WARN_ADD([-Wunused-but-set-parameter])
UL_WARN_ADD([-Wunused-but-set-variable])
UL_WARN_ADD([-Wunused-parameter])
UL_WARN_ADD([-Wunused-result])
UL_WARN_ADD([-Wunused-variable])
UL_WARN_ADD([-Wduplicated-cond])
UL_WARN_ADD([-Wduplicated-branches])
UL_WARN_ADD([-Wrestrict])
UL_WARN_ADD([-Wnull-dereference])

UL_WARN_ADD([-Wno-shadow])
UL_WARN_ADD([-Wno-sign-compare])

AC_SUBST([WARN_CFLAGS])

###### handle configure switches, select dependencies ######

AC_ARG_WITH([tests],
	[AS_HELP_STRING([--without-tests], [Compile test programs])],
	[], [with_tests="yes"])

AC_DEFINE_DIR(TESTBINDIR, libexecdir/mtd-utils,
	      [Path where test and debug programs will be installed])

AC_ARG_ENABLE([ubihealthd],
	[AS_HELP_STRING([--enable-ubihealthd], [Build the ubihealthd program])],
	[], [enable_ubihealthd="yes"])

AC_ARG_WITH([lsmtd],
	[AS_HELP_STRING([--without-lsmtd], [Do not build the lsmtd program])],
	[], [with_lsmtd="yes"])

AC_ARG_WITH([jffs],
	[AS_HELP_STRING([--without-jffs], [Disable jffsX utilities])],
	[], [with_jffs="yes"])

AC_ARG_WITH([ubifs],
	[AS_HELP_STRING([--without-ubifs], [Disable ubifs utilities])],
	[], [with_ubifs="yes"])

AC_ARG_WITH([zlib],
	[AS_HELP_STRING([--with-zlib], [Support zlib deflate compression])],
	[], [with_zlib="check"])

AC_ARG_WITH([xattr],
	[AS_HELP_STRING([--with-xattr], [Support extended file attributes])],
	[], [with_xattr="check"])

AC_ARG_WITH([lzo],
	[AS_HELP_STRING([--with-lzo], [Support LZO compression])],
	[], [with_lzo="check"])

AC_ARG_WITH([zstd],
	[AS_HELP_STRING([--with-zstd], [Support for ZSTD compression])],
	[], [with_zstd="check"])

AC_ARG_WITH([selinux],
	[AS_HELP_STRING([--with-selinux],
		[Support for selinux extended attributes])],
	[], [with_selinux="check"])

AC_ARG_WITH([crypto],
	[AS_HELP_STRING([--with-crypto], [Support for UBIFS crypto features])],
	[], [with_crypto="check"])

##### search for dependencies #####

need_clock_gettime="no"
need_pthread="no"
need_uuid="no"
need_cmocka="no"
need_getrandom="no"

clock_gettime_missing="no"
pthread_missing="no"
uuid_missing="no"
cmocka_missing="no"
getrandom_missing="no"

AS_IF([test "x$enable_unit_tests" = "xyes"], [
	need_cmocka="yes"
])

AS_IF([test "x$with_tests" = "xyes"], [
	need_clock_gettime="yes"
	need_pthread="yes"
])

AS_IF([test "x$enable_ubihealthd" = "xyes"], [
	need_getrandom="yes"
])

AS_IF([test "x$with_ubifs" = "xyes"], [
	need_uuid="yes"
	need_getrandom="yes"
])

AS_IF([test "x$with_zlib" != "xno"], [
	PKG_CHECK_MODULES(ZLIB, [zlib], [with_zlib="yes"],
				[AS_IF([test "x$with_zlib" != "xcheck"],
				       [AC_MSG_ERROR([cannot find zlib])],
				       [with_zlib="no"])])
], [])

if test "x$need_uuid" = "xyes"; then
	PKG_CHECK_MODULES(UUID, [uuid], [], [uuid_missing="yes"])
fi

if test "x$need_clock_gettime" = "xyes"; then
	AC_SEARCH_LIBS([clock_gettime], [rt posix4])
	AC_CHECK_FUNCS([clock_gettime], [], [clock_gettime_missing="yes"])
fi

if test "x$need_pthread" = "xyes"; then
	AX_PTHREAD([], [pthread_missing="yes"])
fi

AC_ARG_VAR([LZO_CFLAGS], [C compiler flags for lzo])
AC_ARG_VAR([LZO_LIBS], [linker flags for lzo])

AS_IF([test -z "$LZO_LIBS" -a "x$with_lzo" != "xno"], [
	AC_CHECK_LIB([lzo2], [lzo1x_1_15_compress], [LZO_LIBS="-llzo2"],
			     [AC_CHECK_LIB([lzo],[lzo1x_1_15_compress],
				[LZO_LIBS="-llzo"],
			     	[]
			     )]
		    )
], [])

AS_IF([test -z "$LZO_LIBS"], [AS_IF([test "x$with_lzo" = "xyes"],
				    [AC_MSG_ERROR([cannot find liblzo])],
				    [with_lzo="no"])],
			     [with_lzo="yes"])

AS_IF([test "x$with_zstd" != "xno"], [
	PKG_CHECK_MODULES(ZSTD, [libzstd], [with_zstd="yes"],
				[AS_IF([test "x$with_zstd" != "xcheck"],
				       [AC_MSG_ERROR([cannot find zstd])],
				       [with_zstd="no"])])
], [])

AS_IF([test "x$with_xattr" != "xno"], [
	have_xattr="yes"

	AC_CHECK_HEADERS([sys/xattr.h], [], [have_xattr="no"])
	AC_CHECK_HEADERS([sys/acl.h], [], [have_xattr="no"])

	AS_IF([test "x$with_xattr" != "xcheck" -a "x$have_xattr" = "xno"],
		      [AC_MSG_ERROR([cannot find xattr/acl headers])],
		      [with_xattr="$have_xattr"])
], [])

AS_IF([test "x$with_selinux" != "xno"], [
	have_selinux="yes"

	PKG_CHECK_MODULES(LIBSELINUX, [libselinux], [], [have_selinux="no"])
	AC_CHECK_HEADERS([selinux/selinux.h], [], [have_selinux="no"])
	AC_CHECK_HEADERS([selinux/label.h], [], [have_selinux="no"])

	AS_IF([test "x$with_selinux" != "xcheck" -a "x$have_selinux" = "xno"],
	      [AC_MSG_ERROR([cannot find SELinux libraries])],
	      [with_selinux="$have_selinux"])
], [])

AS_IF([test "x$with_crypto" != "xno"], [
	have_openssl="yes"

	AC_CHECK_HEADERS([openssl/rand.h], [], [have_openssl="no"])
	PKG_CHECK_MODULES(OPENSSL, [openssl], [], [have_openssl="no"])

	AS_IF([test "x$with_crypto" != "xcheck" -a "x$have_openssl" = "xno"],
	      [AC_MSG_ERROR([cannot find OpenSSL libraries])],
	      [with_crypto="$have_openssl"])
], [])

if test "x$need_getrandom" = "xyes"; then
	AC_CHECK_HEADERS([sys/random.h], [], [getrandom_missing="yes"])
fi

if test "x$need_cmocka" = "xyes"; then
	PKG_CHECK_MODULES(CMOCKA, [cmocka], [], [cmocka_missing="yes"])
fi

AC_CHECK_HEADERS([execinfo.h])

##### produce summary on dependencies #####

dep_missing="no"

if test "x$clock_gettime_missing" = "xyes"; then
	AC_MSG_WARN([cannot find clock_gettime function required for MTD tests])
	AC_MSG_NOTICE([building test programs can optionally be dissabled])
	dep_missing="yes"
fi

if test "x$pthread_missing" = "xyes"; then
	AC_MSG_WARN([cannot find pthread support required for test programs])
	AC_MSG_NOTICE([building test programs can optionally be dissabled])
	dep_missing="yes"
fi

if test "x$uuid_missing" = "xyes"; then
	AC_MSG_WARN([cannot find uuid library required for mkfs.ubifs])
	AC_MSG_NOTICE([mtd-utils can optionally be built without mkfs.ubifs])
	dep_missing="yes"
fi

if test "x$getrandom_missing" = "xyes"; then
	AC_MSG_WARN([cannot find headers for getrandom() function])
	AC_MSG_NOTICE([mkfs.ubifs, ubihealthd can optionally be disabled])
	need_getrandom="no"
fi

if test "x$cmocka_missing" = "xyes"; then
	AC_MSG_WARN([cannot find CMocka library required for unit tests])
	AC_MSG_NOTICE([unit tests can optionally be disabled])
	dep_missing="yes"
fi

if test "x$dep_missing" = "xyes"; then
	AC_MSG_ERROR([missing one or more dependencies])
fi

##### generate output #####

AM_CONDITIONAL([WITH_LZO], [test "x$with_lzo" = "xyes"])
AM_CONDITIONAL([WITH_ZLIB], [test "x$with_zlib" = "xyes"])
AM_CONDITIONAL([WITH_ZSTD], [test "x$with_zstd" = "xyes"])
AM_CONDITIONAL([WITH_XATTR], [test "x$with_xattr" = "xyes"])
AM_CONDITIONAL([WITH_SELINUX], [test "x$with_selinux" = "xyes"])
AM_CONDITIONAL([WITH_CRYPTO], [test "x$with_crypto" = "xyes"])
AM_CONDITIONAL([WITH_UBIHEALTHD], [test "x$enable_ubihealthd" = "xyes"])

AM_CONDITIONAL([BUILD_UBIFS], [test "x$with_ubifs" = "xyes"])
AM_CONDITIONAL([BUILD_JFFSX], [test "x$with_jffs" = "xyes"])
AM_CONDITIONAL([BUILD_LSMTD], [test "x$with_lsmtd" = "xyes"])
AM_CONDITIONAL([BUILD_TESTS], [test "x$with_tests" = "xyes"])
AM_CONDITIONAL([UNIT_TESTS], [test "x$enable_unit_tests" = "xyes"])

AC_CHECK_SIZEOF([off_t])
AC_CHECK_SIZEOF([loff_t])

AC_CONFIG_HEADERS([include/config.h])

AC_CONFIG_FILES([tests/fs-tests/fs_help_all.sh
	tests/fs-tests/fs_run_all.sh
	tests/fs-tests/stress/fs_stress00.sh
	tests/fs-tests/stress/fs_stress01.sh
	tests/ubi-tests/runubitests.sh
	tests/ubi-tests/ubi-stress-test.sh])

AC_OUTPUT([Makefile])

AC_MSG_RESULT([
	${PACKAGE}  ${VERSION}

	prefix:            ${prefix}
	exec prefix:       ${exec_prefix}

	runstatedir:       ${runstatedir}
	bindir:            ${bindir}
	sbindir:           ${sbindir}
	libdir:            ${libdir}
	includedir:        ${includedir}

	compiler:          ${CC}
	cflags:            ${CFLAGS}
	ldflags:           ${LDFLAGS}

	lzo support:       ${with_lzo}
	zlib support:      ${with_zlib}
	zstd support:      ${with_zstd}
	xattr/acl support: ${with_xattr}
	SELinux support:   ${with_selinux}
	fscrypt support:   ${with_crypto}

	Test programs:     ${with_tests}
	Unit tests:        ${enable_unit_tests}
	ubihealthd:        ${enable_ubihealthd}
	lsmtd:             ${with_lsmtd}
	jffs2 utils:       ${with_jffs}
	ubifs utils:       ${with_ubifs}

	warnings:

${WARN_CFLAGS}

	Type 'make' or 'make <utilname>' to compile.
])
