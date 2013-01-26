AC_PREREQ(2.52)

AC_INIT(GobiAPI, 1.0.40, foo@it.com, GobiAPI)
AM_INIT_AUTOMAKE([1.9 subdir-objects tar-ustar no-dist-gzip dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS(config.h)

dnl Required programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_INSTALL
LT_INIT

AC_CONFIG_FILES([
Makefile
Core/Makefile
Database/Makefile
Database/QMI/Makefile
Shared/Makefile
GobiConnectionMgmt/Makefile
GobiImageMgmt/Makefile
GobiQDLService/Makefile
])
AC_OUTPUT

