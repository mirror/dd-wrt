dnl m4/jitcache.m4
dnl
dnl Copyright (C) 2008
dnl CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
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


dnl check for jitcache support
AC_DEFUN([AC_CHECK_ENABLE_JITCACHE],[
AC_MSG_CHECKING(whether JIT compiler output caching should be enabled)
AC_ARG_ENABLE([jitcache],
              [AS_HELP_STRING(--enable-jitcache,enable caching of JIT compiler output [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_JITCACHE=yes;;
                   *) ENABLE_JITCACHE=no;;
               esac],
              [ENABLE_JITCACHE=no])
AC_MSG_RESULT(${ENABLE_JITCACHE})
AM_CONDITIONAL([ENABLE_JITCACHE], test x"${ENABLE_JITCACHE}" = "xyes")

if test x"${ENABLE_JITCACHE}" = "xyes"; then
    AC_DEFINE([ENABLE_JITCACHE], 1, [store and load JIT compiler output])
fi
])
