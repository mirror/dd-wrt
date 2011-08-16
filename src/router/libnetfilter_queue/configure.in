dnl Process this file with autoconf to create configure.

AC_INIT
AC_CANONICAL_SYSTEM
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(libnetfilter_queue, 1.0.0)

AC_PROG_CC
AC_EXEEXT
AM_PROG_LIBTOOL
AC_PROG_INSTALL

AC_SUBST(LIBTOOL_DEPS)

case $target in 
*-*-linux*) ;;
*) AC_MSG_ERROR([Linux only, dude!]);;
esac

dnl Dependencies
LIBNFNETLINK_REQUIRED=0.0.41
 
PKG_CHECK_MODULES(LIBNFNETLINK, libnfnetlink >= $LIBNFNETLINK_REQUIRED,,
	AC_MSG_ERROR(Cannot find libnfnetlink >= $LIBNFNETLINK_REQUIRED))
 
CFLAGS="$CFLAGS $LIBNFNETLINK_CFLAGS"
LIBNFQUEUE_LIBS="$LIBNFNETLINK_LIBS"
 
AC_SUBST(LIBNFQUEUE_LIBS)

dnl Output the makefiles
AC_OUTPUT(Makefile include/Makefile include/libnetfilter_queue/Makefile src/Makefile utils/Makefile libnetfilter_queue.pc doxygen.cfg)
