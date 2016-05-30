dnl m4/disassembler.m4
dnl
dnl Copyright (C) 1996-2013
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


dnl check for disassembler support

AC_DEFUN([AC_CHECK_ENABLE_DISASSEMBLER],[
AC_MSG_CHECKING(whether disassembler should be enabled)
AC_ARG_ENABLE([disassembler],
              [AS_HELP_STRING(--enable-disassembler,enable disassembler [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_DISASSEMBLER=yes;;
                   *) ENABLE_DISASSEMBLER=no;;
               esac],
              [ENABLE_DISASSEMBLER=no])
AC_MSG_RESULT(${ENABLE_DISASSEMBLER})
AM_CONDITIONAL([ENABLE_DISASSEMBLER], test x"${ENABLE_DISASSEMBLER}" = "xyes")

if test x"${ENABLE_DISASSEMBLER}" = "xyes"; then
    AC_DEFINE([ENABLE_DISASSEMBLER], 1, [enable disassembler])

    dnl check for binutils headers and libraries on some architectures for the
    dnl disassembler
    case "${ARCH_DIR}" in
        arm | i386 | mips | powerpc | x86_64 | sparc64 | powerpc64 | s390)
            AC_CHECK_HEADER([ansidecl.h],, [AC_MSG_ERROR(cannot find ansidecl.h)])
            AC_CHECK_HEADER([symcat.h],, [AC_MSG_ERROR(cannot find symcat.h)])
            AC_CHECK_HEADER([bfd.h],, [AC_MSG_ERROR(cannot find bfd.h)])
            AC_CHECK_HEADER([dis-asm.h],, [AC_MSG_ERROR(cannot find dis-asm.h)])

            case "${OS_DIR}" in
                cygwin | darwin | netbsd | solaris)
                     AC_CHECK_LIB(intl, dcgettext,, [AC_MSG_ERROR(cannot find libintl (from binutils))])
                     ;;
            esac

            case "${OS_DIR}" in
                solaris)
					 AC_CHECK_LIB(rt, sem_init,, [AC_MSG_ERROR(cannot find librt)])
                     ;;
            esac


            AC_CHECK_LIB(iberty, xstrerror,, [AC_MSG_ERROR(cannot find libiberty (from binutils))])
            AC_CHECK_LIB(bfd, bfd_get_arch,, [AC_MSG_ERROR(cannot find libbfd (from binutils))])
            AC_CHECK_LIB(opcodes, disassembler,, [AC_MSG_ERROR(cannot find libopcodes (from binutils))])
            AC_DEFINE([WITH_BINUTILS_DISASSEMBLER], 1, [use binutils disassembler])
            AM_CONDITIONAL([WITH_BINUTILS_DISASSEMBLER], [true])
            ;;
        * )
            AM_CONDITIONAL([WITH_BINUTILS_DISASSEMBLER], [false])
            ;;
    esac
else
    AM_CONDITIONAL([WITH_BINUTILS_DISASSEMBLER], [false])
fi
])
