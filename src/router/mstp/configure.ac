AC_PREREQ([2.61])
AC_INIT([mstpd], [0.1.0], [https://github.com/mstpd/mstpd])

# bridge-stp must be installed in /sbin/, and the associated scripts and
# executables should be installed outside of /usr/ by default.  If `./configure`
# was run without specifying `--exec-prefix=...`, then set the default
# exec_prefix so these files will be installed in the correct paths by default.
# AC_PREFIX_DEFAULT(...) can be used to override the default prefix, but there
# doesn't seem to be any standard macro to override the default exec_prefix, so
# just do it manually.
test "x$exec_prefix" = xNONE && exec_prefix=''

# bridge-stp.conf should be installed in /etc/ rather than /usr/local/etc/ by
# default.  If `./configure` was run without specifying `--prefix=...` or
# `--sysconfdir=...`, then set the default sysconfdir appropriately.
# There is no standard macro to override the default sysconfdir, so just do it
# manually.
test "x$prefix" = xNONE && test "x$sysconfdir" = 'x${prefix}/etc' && sysconfdir='/etc'

# The mstpd PID file should not be installed under /usr/ if mstpd itself is not
# installed under /usr.  If exec_prefix is empty and `./configure` was run
# without specifying `--localstatedir=...`, then set the default localstatedir
# appropriately.
# There is no standard macro to override the default localstatedir, so just do
# it manually.
test "x$exec_prefix" = x && test "x$localstatedir" = 'x${prefix}/var' && localstatedir='/var'

# If exec_prefix is empty, then the default libexecdir is normally /libexec/,
# which is not a standard path.  Therefore, if exec_prefix is empty and
# `./configure` was run without specifying `--libexecdir=...`, set the default
# localstatedir to /lib/ instead of /libexec/.
# There is no standard macro to override the default libexecdir, so just do it
# manually.
test "x$exec_prefix" = x && test "x$libexecdir" = 'x${exec_prefix}/libexec' && libexecdir='/lib'

AM_INIT_AUTOMAKE([foreign -Wall])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT

PKG_PROG_PKG_CONFIG

# Optional building of examples
AC_ARG_ENABLE([devel],
	[AC_HELP_STRING([--enable-devel], [build devel mode])])

AM_CONDITIONAL([ENABLE_DEVEL], [test "x$enable_devel" = "xyes"])

AC_ARG_ENABLE([install-ifupdown-scripts],
	[AC_HELP_STRING([--enable-install-ifupdown-scripts], [enable installation of ifupdown scripts])])

AM_CONDITIONAL([ENABLE_INSTALL_IFUPDOWN_SCRIPTS], [test "x$enable_install_ifupdown_scripts" = "xyes"])

AC_ARG_WITH([bashcompletiondir],
     [AS_HELP_STRING([--with-bashcompletiondir=DIR], [Directory for bash completion files.
     [default=${sysconfdir}/bash_completion.d]])],,
     [with_bashcompletiondir=default])
AS_IF([test "x$with_bashcompletiondir" = "xdefault"],
     [AC_SUBST([bashcompletiondir], [${sysconfdir}/bash_completion.d])])
AS_IF([test "x$with_bashcompletiondir" != "xdefault"],
      [AC_SUBST([bashcompletiondir], [$with_bashcompletiondir])])

AC_ARG_WITH([systemdunitdir],
     [AS_HELP_STRING([--with-systemunitdir=DIR], [Directory for systemd unit files.])],,
     [with_systemdunitdir=default])
AS_IF([test "x$with_systemdunitdir" = "xdefault"],
     [AC_SUBST([systemdunitdir], [$($PKG_CONFIG --variable=systemdsystemunitdir systemd 2> /dev/null)])])
AS_IF([test "x$with_systemdunitdir" != "xdefault"],
      [AC_SUBST([systemdunitdir], [$with_systemdunitdir])])

# Define PACKAGE_BUILD
PACKAGE_BUILD=`git log --pretty=format:'%h' -n 1`
AC_SUBST(PACKAGE_BUILD)
AC_DEFINE_UNQUOTED(PACKAGE_BUILD, "$PACKAGE_BUILD", [Build revision obtained from git])

# Override PACKAGE_VERSION to include git tag
PACKAGE_VERSION="${PACKAGE_VERSION}-${PACKAGE_BUILD}"
AC_SUBST(PACKAGE_VERSION)
AC_DEFINE_UNQUOTED(PACKAGE_VERSION, "$PACKAGE_VERSION", [Package version, including build number])

AC_SEARCH_LIBS([clock_gettime], [rt])

AC_CHECK_TYPES(struct timespec)
AC_CHECK_FUNCS(clock_gettime)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
