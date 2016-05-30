dnl m4/vm-zip.m4
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


dnl where is CACAO's vm.zip

AC_DEFUN([AC_CHECK_WITH_VM_ZIP],[
AC_MSG_CHECKING(where CACAO's vm.zip is installed)
AC_ARG_WITH([vm-zip],
            [AS_HELP_STRING(--with-vm-zip=<path>,path to CACAO's vm.zip (includes the name of the file and may be flat) [[default=(--enable-zlib:${CACAO_PREFIX}/share/cacao/vm.zip,--disable-zlib:${CACAO_PREFIX}/share/cacao/classes)]])],
            [CACAO_VM_ZIP=${withval}],
            [case "${ENABLE_ZLIB}" in
                 no)
                     CACAO_VM_ZIP=${CACAO_PREFIX}/share/cacao/classes
                     ;;
                 *)
                     CACAO_VM_ZIP=${CACAO_PREFIX}/share/cacao/vm.zip
                     ;;
             esac
            ])
AC_MSG_RESULT(${CACAO_VM_ZIP})
AC_DEFINE_UNQUOTED([CACAO_VM_ZIP], "${CACAO_VM_ZIP}", [CACAO's vm.zip])
AC_SUBST(CACAO_VM_ZIP)
])
