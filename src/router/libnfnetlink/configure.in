dnl Process this file with autoconf to create configure.

AC_INIT(libnfnetlink, 1.0.0)

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE

AC_PROG_CC
AC_EXEEXT
AM_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

case $target in 
*-*-linux*) ;;
*) AC_MSG_ERROR([Linux only, dude!]);;
esac


dnl--------------------------------
dnl--------------------------------


dnl Output the makefile
AC_OUTPUT(Makefile src/Makefile include/Makefile include/libnfnetlink/Makefile utils/Makefile libnfnetlink.pc)
