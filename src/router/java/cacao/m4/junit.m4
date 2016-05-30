dnl m4/junit.m4
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


dnl Check location of JUnit jar file.

AC_DEFUN([AC_CHECK_WITH_JUNIT_JAR],[
AC_MSG_CHECKING(location of JUnit jar)
AC_ARG_WITH([junit-jar],
            [AS_HELP_STRING(--with-junit-jar=<path>,location of JUnit jar file [[default=/usr/share/java/junit4.jar]])],
            [JUNIT_JAR="${withval}"],
            [JUNIT_JAR="/usr/share/java/junit4.jar"])
AC_MSG_RESULT(${JUNIT_JAR})
AC_SUBST(JUNIT_JAR)
])

AC_DEFUN([AC_CHECK_JUNIT_WORKS],[
AC_CACHE_CHECK([if JUnit works], ac_cv_prog_junit_works, [
JAVA_TEST=JUnitTest.java
CLASS_TEST=JUnitTest.class
cat << \EOF > $JAVA_TEST
/* [#]line __oline__ "configure" */
import org.junit.Assert;
import org.junit.Test;
public class JUnitTest {
    @Test
    public void testTrue() {
        Assert.assertTrue(true);
    }
}
EOF
if AC_TRY_COMMAND($JAVAC -source 1.5 -target 1.5 -classpath ${JUNIT_JAR} $JAVA_TEST) >/dev/null 2>&1; then
  ac_cv_prog_junit_works=yes
else
  AC_MSG_ERROR([The Java compiler $JAVAC failed to compile JUnit tests (see config.log, check the CLASSPATH?)])
  echo "configure: failed program was:" >&AC_FD_CC
  cat $JAVA_TEST >&AC_FD_CC
fi
rm -f $JAVA_TEST $CLASS_TEST
])
])
