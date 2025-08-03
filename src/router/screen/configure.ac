AC_INIT([GNU Screen],[5.0.1],[https://savannah.gnu.org/bugs/?group=screen],[screen],[https://www.gnu.org/software/screen/])
AC_PREREQ([2.71])

AC_CONFIG_HEADERS([config.h])


AC_DEFINE_UNQUOTED([VERSION_MAJOR], 
	[`echo $PACKAGE_VERSION | cut -d . -f 1`],
	[Major version of GNU screen])
AC_DEFINE_UNQUOTED([VERSION_MINOR],
	[`echo $PACKAGE_VERSION | cut -d . -f 2 | cut -d - -f 1`],
	[Minor version of GNU screen])
AC_DEFINE_UNQUOTED([VERSION_REVISION],
	[`echo $PACKAGE_VERSION | cut -d . -f 3 | cut -d - -f 1`],
	[Micro revision of GNU screen])


AH_TEMPLATE([HAVE_EXECVPE], [system has execvpe() defined])
AH_TEMPLATE([HAVE_OPENPTY], [system has openpty() defined])

dnl -
AH_TEMPLATE([ENABLE_UTMP], [Enable utmp support])
AH_TEMPLATE([HAVE_UTEMPTER], [system has utempter helper binary])
AH_TEMPLATE([HAVE_UT_EXIT], [utmpx struct has ut_exit field])
AH_TEMPLATE([HAVE_UT_HOST], [utmpx struct has ut_host field])

dnl -
AH_TEMPLATE([ENABLE_PAM], [Enable Pluggable Authentication Modules support])

dnl -
AH_TEMPLATE([ENABLE_TELNET], [Enable built-in telnet client])

dnl -
AH_TEMPLATE([SOCKET_DIR], [Path to socket directory])

dnl -
AH_TEMPLATE([SYSTEM_SCREENRC], [Path to system screenrc])

dnl - pty settings
AH_TEMPLATE([PTY_MODE], [Default mode for pty devices])
AH_TEMPLATE([PTY_GROUP], [Default group for pty devices])
AH_TEMPLATE([PTY_ROFS], [pty devices are on read only filesystem])


dnl
dnl Check for sane compiler
dnl

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL
AC_USE_SYSTEM_EXTENSIONS

dnl
dnl Check various system stuff
dnl

dnl check for execvpe()
AC_CHECK_FUNCS([execvpe])

dnl
AC_CHECK_FUNCS([seteuid setegid setreuid setresuid])

dnl curses compatible lib, we do forward declaration ourselves, only need to link to proper library
AC_SEARCH_LIBS([tgetent], [curses termcap termlib ncursesw tinfow ncurses tinfo], [], [
	AC_MSG_ERROR([unable to find tgetent() function])
])

dnl check for crypt()
AC_SEARCH_LIBS([crypt], [crypt], [], [
	AC_MSG_ERROR([unable to find crypt() function])
])

dnl
AC_CHECK_HEADERS(pty.h util.h libutil.h)
AC_SEARCH_LIBS([openpty], [util openpty bsd], [
	AC_DEFINE(HAVE_OPENPTY)
], [
	AC_MSG_ERROR([unable to find openpty() function])
])

dnl
AC_SEARCH_LIBS([socket], [network], [], [
	AC_MSG_ERROR([unable to find socket() function])
])

dnl
AC_CHECK_HEADERS(langinfo.h)

dnl
AC_CHECK_HEADERS(dirent.h)

dnl
dnl Check user changeable stuff
dnl

AC_ARG_ENABLE(pam, AS_HELP_STRING([--enable-pam],
	      [enable PAM support (default: enabled)]),
	      [enable_pam=$enableval],
	      [enable_pam=yes])
AC_ARG_ENABLE(utmp, AS_HELP_STRING([--enable-utmp],
	      [enable utmp support (default: disabled)]),
	      [enable_utmp=$enableval],
	      [enable_utmp=no])
AC_ARG_ENABLE(telnet, AS_HELP_STRING([--enable-telnet],
	      [enable telnet support (default: disabled)]),
	      [enable_telnet=$enableval],
	      [enable_telnet=no])
AC_ARG_ENABLE(socket-dir, AS_HELP_STRING([--enable-socket-dir],
	      [enable global socket directory (default: disabled)]),
	      [enable_socket_dir=$enableval],
	      [enable_socket_dir=no])

AC_ARG_WITH(system_screenrc, AS_HELP_STRING([--with-system_screenrc],
	    [set location of system screenrc (default: /etc/screenrc)]),
	    [with_system_screenrc=$withval],
	    [with_system_screenrc=/etc/screenrc])
AC_ARG_WITH(pty-mode, AS_HELP_STRING([--with-pty-mode],
	    [set pty mode (default: 0620)]),
	    [with_pty_mode=$withval],
	    [with_pty_mode=0620])
AC_ARG_WITH(pty-group, AS_HELP_STRING([--with-pty-group],
	    [set pty group (default: 5)]),
	    [with_pty_group=$withval],
	    [with_pty_group=5])
AC_ARG_WITH(pty-rofs, AS_HELP_STRING([--with-pty-rofs],
	    [set rofs handling (default: no)]),
	    [with_pty_rofs=$withval],
	    [with_pty_rofs=no])

dnl -- enable_pam

AS_IF([test "x$enable_pam" = "xyes"], [
	AC_MSG_CHECKING(for PAM support)
	oldlibs="$LIBS"
	LIBS="$LIBS -lpam"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <security/pam_appl.h>]], [[
		pam_start(0, 0, 0, 0);
		pam_authenticate(0, 0);
		pam_end(0,0);
	]])],[AC_MSG_RESULT(yes);
	AC_DEFINE(ENABLE_PAM)],[AC_MSG_ERROR(no);LIBS="$oldlibs"])
])

dnl -- enable_utmp

AS_IF([test "x$enable_utmp" = "xyes"], [
	AC_CHECK_HEADERS(utmpx.h, have_utmpx=yes, have_utmpx=no)
	AS_IF([test "x$have_utmpx" != "xyes"],
	      AC_MSG_ERROR(Missing utmpx.h))

	AC_CHECK_HEADER(utempter.h, have_utempter=yes, have_utempter=no)
	AS_IF([test "x$have_utempter" = "xyes"], [
		AC_DEFINE(HAVE_UTEMPTER)
		LIBS="$LIBS -lutempter"
	])

	AC_MSG_CHECKING(if utmpx struct has ut_host field)
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <utmpx.h>]], [[
		struct utmpx u;
		u.ut_host[0] = 0;
	]])],[AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_UT_HOST)],[AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if utmpx struct has ut_exit field)
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <utmpx.h>]], [[
		struct utmpx u;
		u.ut_exit.e_exit;
	]])],[AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_UT_EXIT)],[AC_MSG_RESULT(no)])

	AC_DEFINE(ENABLE_UTMP)
])

dnl -- enable_telnet

AS_IF([test "x$enable_telnet" = "xyes"], [
	AC_DEFINE(ENABLE_TELNET)
])

dnl -- enable_socket_dir

AS_IF([test "x$enable_socket_dir" != "xno"], [
	AS_IF([test "x$enable_socket_dir" = "xyes"],
	      [enable_socket_dir="/run/screen"]
	)
	AC_DEFINE_UNQUOTED(SOCKET_DIR, "$enable_socket_dir")
])

dnl -- with_sysscreenrc
AC_DEFINE_UNQUOTED(SYSTEM_SCREENRC, "$with_system_screenrc")

dnl -- with_pty_mode
AC_DEFINE_UNQUOTED(PTY_MODE, $with_pty_mode)

dnl -- with_pty_group
AC_DEFINE_UNQUOTED(PTY_GROUP, $with_pty_group)

dnl -- with_pty_rofs
AS_IF([test "x$with_pty_rofs" = "xyes"], [
	AC_DEFINE(PTY_ROFS)
])

dnl
dnl Generate files from *.in ones
dnl

AC_CONFIG_FILES([Makefile doc/Makefile])
AC_OUTPUT

dnl
dnl Print out config
dnl

echo ""
echo "Configuration:"
echo ""
echo " PAM support: .............................. $enable_pam"
echo " telnet support: ........................... $enable_telnet"
echo " utmp support: ............................. $enable_utmp"
echo " global socket directory: .................. $enable_socket_dir"
echo ""
echo " system screenrc location: ................. $with_system_screenrc"
echo " pty mode: ................................. $with_pty_mode"
echo " pty group: ................................ $with_pty_group"
echo " pty on read only file system: ............. $with_pty_rofs"
echo ""

AS_IF([test "x$enable_pam" != "xyes"], [
	echo "!!! WARNING !!!"
	echo "YOU ARE DISABLING PAM SUPPORT!"
	echo "FOR screen TO WORK IT WILL NEED TO RUN AS SUID root BINARY"
	echo "THIS CONFIGURATION IS _HIGHLY_ NOT RECOMMENDED!"
	echo "!!! WARNING !!!"
])
