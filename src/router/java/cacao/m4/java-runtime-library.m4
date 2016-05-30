dnl m4/java-runtime-library.m4
dnl
dnl Copyright (C) 1996-2012
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


dnl which Java runtime library should we use

AC_DEFUN([AC_CHECK_WITH_JAVA_RUNTIME_LIBRARY],[
AC_MSG_CHECKING(which Java runtime library to use)
AC_ARG_WITH([java-runtime-library],
            [AS_HELP_STRING(--with-java-runtime-library=<type>,specifies which type of classpath to use as Java runtime library (cldc1.1,gnuclasspath,openjdk,openjdk7) [[default=gnuclasspath]])],
            [case "${withval}" in
                cldc1.1)
                    WITH_JAVA_RUNTIME_LIBRARY=cldc1.1
                    AC_DEFINE([WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1], 1, [use Sun's CLDC1.1 classes])
                    AC_SUBST(WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1)
                    ;;
                gnuclasspath)
                    WITH_JAVA_RUNTIME_LIBRARY=gnuclasspath
                    AC_DEFINE([WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH], 1, [use GNU Classpath])
                    AC_SUBST(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
                    ;;
                openjdk7)
                    WITH_JAVA_RUNTIME_LIBRARY=openjdk7
                    AC_DEFINE([WITH_JAVA_RUNTIME_LIBRARY_OPENJDK], 1, [use OpenJDK's Java SE classes])
                    AC_DEFINE([WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7], 1, [use OpenJDK's version 7])
                    AC_SUBST(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
                    AC_SUBST(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7)
                    ;;
                openjdk)
                    WITH_JAVA_RUNTIME_LIBRARY=openjdk
                    AC_DEFINE([WITH_JAVA_RUNTIME_LIBRARY_OPENJDK], 1, [use OpenJDK's Java SE classes])
                    AC_SUBST(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
                    ;;
                *)
                    AC_MSG_ERROR(unknown classpath ${withval})
                    ;;
             esac],
            [WITH_JAVA_RUNTIME_LIBRARY=gnuclasspath
             AC_DEFINE([WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH], 1, [use GNU Classpath])
             AC_SUBST(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)])
AC_MSG_RESULT(${WITH_JAVA_RUNTIME_LIBRARY})
AM_CONDITIONAL([WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1], test x"${WITH_JAVA_RUNTIME_LIBRARY}" = "xcldc1.1")
AM_CONDITIONAL([WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH], test x"${WITH_JAVA_RUNTIME_LIBRARY}" = "xgnuclasspath")
AM_CONDITIONAL([WITH_JAVA_RUNTIME_LIBRARY_OPENJDK], test x"${WITH_JAVA_RUNTIME_LIBRARY}" = "xopenjdk" -o x"${WITH_JAVA_RUNTIME_LIBRARY}" = "xopenjdk7")
AM_CONDITIONAL([WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7], test x"${WITH_JAVA_RUNTIME_LIBRARY}" = "xopenjdk7")
])


dnl where is Java runtime library installed

AC_DEFUN([AC_CHECK_WITH_JAVA_RUNTIME_LIBRARY_PREFIX],[
AC_MSG_CHECKING(where Java runtime library is installed)
AC_ARG_WITH([java-runtime-library-prefix],
            [AS_HELP_STRING(--with-java-runtime-library-prefix=<dir>,installation directory of Java runtime library [[default=/usr/local/classpath]])],
            [JAVA_RUNTIME_LIBRARY_PREFIX=${withval}],
            [JAVA_RUNTIME_LIBRARY_PREFIX=/usr/local/classpath])
AC_MSG_RESULT(${JAVA_RUNTIME_LIBRARY_PREFIX})
AC_DEFINE_UNQUOTED([JAVA_RUNTIME_LIBRARY_PREFIX], "${JAVA_RUNTIME_LIBRARY_PREFIX}", [Java runtime library installation directory])
AC_SUBST(JAVA_RUNTIME_LIBRARY_PREFIX)
])


dnl where are Java runtime library classes installed

AC_DEFUN([AC_CHECK_WITH_JAVA_RUNTIME_LIBRARY_CLASSES],[
AC_MSG_CHECKING(where Java runtime library classes are installed)
AC_ARG_WITH([java-runtime-library-classes],
            [AS_HELP_STRING(--with-java-runtime-library-classes=<path>,path to Java runtime library classes (includes the name of the file and may be flat) [[default=(gnuclasspath:${JAVA_RUNTIME_LIBRARY_PREFIX}/share/classpath/glibj.zip,openjdk:${JAVA_RUNTIME_LIBRARY_PREFIX}/control/build/${OS_DIR}-${JAVA_ARCH}/classes,*:${JAVA_RUNTIME_LIBRARY_PREFIX})]])],
            [JAVA_RUNTIME_LIBRARY_CLASSES=${withval}],
            [case "${WITH_JAVA_RUNTIME_LIBRARY}" in
                 gnuclasspath)
                     JAVA_RUNTIME_LIBRARY_CLASSES=${JAVA_RUNTIME_LIBRARY_PREFIX}/share/classpath/glibj.zip
                     ;;
                 openjdk)
                     JAVA_RUNTIME_LIBRARY_CLASSES=${JAVA_RUNTIME_LIBRARY_PREFIX}/control/build/${OS_DIR}-${JAVA_ARCH}/classes
                     ;;
                 *)
                     JAVA_RUNTIME_LIBRARY_CLASSES=${JAVA_RUNTIME_LIBRARY_PREFIX}
                     ;;
             esac])
AC_MSG_RESULT(${JAVA_RUNTIME_LIBRARY_CLASSES})
AC_DEFINE_UNQUOTED([JAVA_RUNTIME_LIBRARY_CLASSES], "${JAVA_RUNTIME_LIBRARY_CLASSES}", [Java runtime library classes])
AC_SUBST(JAVA_RUNTIME_LIBRARY_CLASSES)
])


dnl where are Java core library classes located at compilation time

AC_DEFUN([AC_CHECK_WITH_BUILD_JAVA_RUNTIME_LIBRARY_CLASSES],[
AC_MSG_CHECKING(where Java core library classes are located at compile time)
AC_ARG_WITH([build-java-runtime-library-classes],
            [AS_HELP_STRING(--with-build-java-runtime-library-classes=<path>,path to Java core library classes (includes the name of the file and may be flat) [[default=${JAVA_RUNTIME_LIBRARY_CLASSES}]])],
            [BUILD_JAVA_RUNTIME_LIBRARY_CLASSES=${withval}],
            [BUILD_JAVA_RUNTIME_LIBRARY_CLASSES=${JAVA_RUNTIME_LIBRARY_CLASSES}])
AC_MSG_RESULT(${BUILD_JAVA_RUNTIME_LIBRARY_CLASSES})
AC_DEFINE_UNQUOTED([BUILD_JAVA_RUNTIME_LIBRARY_CLASSES], "${BUILD_JAVA_RUNTIME_LIBRARY_CLASSES}", [Java core library classes at compile time])
AC_SUBST(BUILD_JAVA_RUNTIME_LIBRARY_CLASSES)

dnl define BOOTCLASSPATH for Makefiles
case "${WITH_JAVA_RUNTIME_LIBRARY}" in
    cldc1.1 | gnuclasspath)
        BOOTCLASSPATH="\$(top_builddir)/src/classes/classes:${BUILD_JAVA_RUNTIME_LIBRARY_CLASSES}"
        ;;
    *)
        BOOTCLASSPATH="${BUILD_JAVA_RUNTIME_LIBRARY_CLASSES}"
        ;;
esac
AC_SUBST(BOOTCLASSPATH)
])


dnl where are Java runtime library native libraries installed

AC_DEFUN([AC_CHECK_WITH_JAVA_RUNTIME_LIBRARY_LIBDIR],[
AC_MSG_CHECKING(where Java runtime library native libraries are installed)
AC_ARG_WITH([java-runtime-library-libdir],
            [AS_HELP_STRING(--with-java-runtime-library-libdir=<dir>,installation directory of Java runtime library native libraries [[default=(gnuclasspath:${JAVA_RUNTIME_LIBRARY_PREFIX}/lib,openjdk:${JAVA_RUNTIME_LIBRARY_PREFIX}/control/build/${OS_DIR}-${JAVA_ARCH}/lib/${JAVA_ARCH},*:${JAVA_RUNTIME_LIBRARY_PREFIX})]])],
            [JAVA_RUNTIME_LIBRARY_LIBDIR=${withval}],
            [case "${WITH_JAVA_RUNTIME_LIBRARY}" in
                 gnuclasspath)
                     JAVA_RUNTIME_LIBRARY_LIBDIR=${JAVA_RUNTIME_LIBRARY_PREFIX}/lib
                     ;;
                 openjdk*)
                     JAVA_RUNTIME_LIBRARY_LIBDIR=${JAVA_RUNTIME_LIBRARY_PREFIX}/control/build/${OS_DIR}-${JAVA_ARCH}/lib/${JAVA_ARCH}
                     ;;
                 *)
                     JAVA_RUNTIME_LIBRARY_LIBDIR=${JAVA_RUNTIME_LIBRARY_PREFIX}
                     ;;
             esac])
AC_MSG_RESULT(${JAVA_RUNTIME_LIBRARY_LIBDIR})

dnl expand JAVA_RUNTIME_LIBRARY_LIBDIR to something that is usable in C code
AS_AC_EXPAND([JAVA_RUNTIME_LIBRARY_LIBDIR], ${JAVA_RUNTIME_LIBRARY_LIBDIR})
AC_DEFINE_UNQUOTED([JAVA_RUNTIME_LIBRARY_LIBDIR], "${JAVA_RUNTIME_LIBRARY_LIBDIR}", [Java runtime library native libraries installation directory])
AC_SUBST(JAVA_RUNTIME_LIBRARY_LIBDIR)
])


dnl where jni_md.h is installed

AC_DEFUN([AC_CHECK_WITH_JNI_MD_H],[
AC_MSG_CHECKING(where jni_md.h is installed)
AC_ARG_WITH([jni_md_h],
            [AS_HELP_STRING(--with-jni_md_h=<dir>,path to jni_md.h [[default=(openjdk:${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/solaris/javavm/export,*:${JAVA_RUNTIME_LIBRARY_PREFIX}/include)]])],
            [WITH_JNI_MD_H=${withval}],
            [case "${WITH_JAVA_RUNTIME_LIBRARY}" in
                 openjdk*)
                     WITH_JNI_MD_H=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/solaris/javavm/export
                     ;;
                 *)
                     WITH_JNI_MD_H=${JAVA_RUNTIME_LIBRARY_PREFIX}/include
                     ;;
            esac])
AC_MSG_RESULT(${WITH_JNI_MD_H})

dnl We use CPPFLAGS so jni.h can find jni_md.h
CPPFLAGS="${CPPFLAGS} -I${WITH_JNI_MD_H}"

AC_CHECK_HEADER([${WITH_JNI_MD_H}/jni_md.h],
                [AC_DEFINE_UNQUOTED([INCLUDE_JNI_MD_H], "${WITH_JNI_MD_H}/jni_md.h", [Java runtime library jni_md.h header])],
                [AC_MSG_ERROR(cannot find jni_md.h)])
])


dnl where jni.h is installed

AC_DEFUN([AC_CHECK_WITH_JNI_H],[
AC_MSG_CHECKING(where jni.h is installed)
AC_ARG_WITH([jni_h],
            [AS_HELP_STRING(--with-jni_h=<dir>,path to jni.h [[default=(openjdk:${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/share/javavm/export,*:${JAVA_RUNTIME_LIBRARY_PREFIX}/include)]])],
            [WITH_JNI_H=${withval}],
            [case "${WITH_JAVA_RUNTIME_LIBRARY}" in
                 openjdk*)
                     WITH_JNI_H=${JAVA_RUNTIME_LIBRARY_PREFIX}/jdk/src/share/javavm/export
                     ;;
                 *)
                     WITH_JNI_H=${JAVA_RUNTIME_LIBRARY_PREFIX}/include
                     ;;
            esac])
AC_MSG_RESULT(${WITH_JNI_H})

dnl We use CPPFLAGS so jni.h can find jni_md.h
CPPFLAGS="${CPPFLAGS} -I${WITH_JNI_H}"

AC_CHECK_HEADER([${WITH_JNI_H}/jni.h],
                [AC_DEFINE_UNQUOTED([INCLUDE_JNI_H], "${WITH_JNI_H}/jni.h", [Java runtime library jni.h header])],
                [AC_MSG_ERROR(cannot find jni.h)],
                [#include INCLUDE_JNI_MD_H])
])
