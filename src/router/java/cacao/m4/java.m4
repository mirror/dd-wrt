dnl m4/java.m4
dnl
dnl Copyright (C) 2007 R. Grafl, A. Krall, C. Kruegel,
dnl C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
dnl E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
dnl J. Wenninger, Institut f. Computersprachen - TU Wien
dnl 
dnl This file is part of CACAO.
dnl 
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 2, or (at
dnl your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
dnl 02110-1301, USA.


dnl check which Java configuration to use

AC_DEFUN([AC_CHECK_ENABLE_JAVA],[
AC_MSG_CHECKING(which Java configuration to use)
AC_ARG_ENABLE([java],
              [AS_HELP_STRING(--enable-java,enable specific Java configuration (cldc1.1,javase) [[default=javase]])],
              [case "${enableval}" in
                   cldc1.1)
                       ENABLE_JAVAME_CLDC1_1=yes
                       AC_DEFINE([ENABLE_JAVAME_CLDC1_1], 1, [compile for Java ME CLDC1.1])
                       AC_MSG_RESULT(cldc1.1)
                       ;;
                   javase)
                       ENABLE_JAVASE=yes
                       AC_DEFINE([ENABLE_JAVASE], 1, [compile for Java SE])
                       AC_MSG_RESULT(javase)
                       ;;
                   *)
                       AC_MSG_ERROR(${enableval} is an unknown configuration)
                       ;;
               esac],
               [ENABLE_JAVASE=yes
                AC_DEFINE([ENABLE_JAVASE], 1, [compile for Java SE])
                AC_MSG_RESULT(j2se)])
AM_CONDITIONAL([ENABLE_JAVAME_CLDC1_1], test x"${ENABLE_JAVAME_CLDC1_1}" = "xyes")
AM_CONDITIONAL([ENABLE_JAVASE], test x"${ENABLE_JAVASE}" = "xyes")
])
