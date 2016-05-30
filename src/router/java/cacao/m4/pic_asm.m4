dnl m4/pic_asm.m4
dnl
dnl Copyright (C) 2010
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


AC_DEFUN([AC_CHECK_ENABLE_PIC_ASM],[
AC_MSG_CHECKING([if we should use position independent assembler code])
AC_ARG_ENABLE([pic-asm],
              [AS_HELP_STRING(--enable-pic-asm,[use position independent assembler code, needed for Solaris i386] [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_PIC_ASM=yes;;
                   *) ENABLE_PIC_ASM=no;;
               esac],
              [ENABLE_PIC_ASM=no])
AC_MSG_RESULT(${ENABLE_PIC_ASM})
AM_CONDITIONAL([ENABLE_PIC_ASM], [test x"${ENABLE_PIC_ASM}" = "xyes"])
])
