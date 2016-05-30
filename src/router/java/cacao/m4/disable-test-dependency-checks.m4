dnl m4/disable-test-dependency-checks.m4
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


dnl check if we should skip checking for test dependencies

AC_DEFUN([AC_CHECK_DISABLE_TEST_DEPENDENCY_CHECKS],[
AC_MSG_CHECKING(whether to disable checking for test dependencies)
AC_ARG_ENABLE([test-dependency-checks],
              [AS_HELP_STRING(--disable-test-dependency-checks, disable checking for test dependencies [[default=no]])],
              [case "${enableval}" in
                   no) disable_test_dependency_checks=yes;;
                   *) disable_test_dependency_checks=no;;
               esac],
              [case "${WITH_JAVA_RUNTIME_LIBRARY}" in
                  cldc1.1 | gnuclasspath)
                      disable_test_dependency_checks=no
                      ;;
                  openjdk | openjdk7)
                      disable_test_dependency_checks=yes
                      ;;
                  *)
                      AC_MSG_ERROR(unknown classpath configuration ${WITH_JAVA_RUNTIME_LIBRARY})
                      ;;
               esac])
AC_MSG_RESULT(${disable_test_dependency_checks})
])
