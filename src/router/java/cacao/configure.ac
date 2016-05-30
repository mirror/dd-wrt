dnl configure.ac
dnl
dnl Copyright (C) 1996-2014
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

dnl Process this file with autoconf to produce a configure script.


AC_INIT(cacao, 1.6.2pre, cacao@cacaojvm.org)
AC_CONFIG_SRCDIR(src/cacao/cacao.cpp)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])
AC_CANONICAL_HOST
AC_PREREQ(2.59)
AM_INIT_AUTOMAKE([1.9.0 dist-xz tar-ustar])
AM_MAINTAINER_MODE

AC_CONFIG_HEADERS([config.h])

AC_PREFIX_DEFAULT(/usr/local/cacao)

dnl
dnl === CAUTION ===
dnl
dnl CFLAGS, CXXFLAGS, CPPFLAGS, LDFLAGS are reserved for the user aka the
dnl person building cacao. Never set one of these variables in configure.ac,
dnl Makefile.am or in m4 macros. Choose their AM_* counterparts instead.
dnl
dnl For more information see [1], [2] and [3].
dnl
dnl [1]: http://www.gnu.org/software/autoconf/manual/html_node/Preset-Output-Variables.html
dnl [2]: http://www.gnu.org/software/automake/manual/html_node/User-Variables.html
dnl [3]: http://www.gnu.org/software/automake/manual/html_node/Flag-Variables-Ordering.html
dnl


dnl Host CPU type.
case "$host_cpu" in
alpha | alphaev56 | alphapca56 )
    ARCH_DIR="alpha"
    AM_CFLAGS="$AM_CFLAGS -mieee"
    AM_CXXFLAGS="$AM_CXXFLAGS -mieee"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__ALPHA__"
    JAVA_ARCH="alpha"
    ;;

arm | armv4* | armv5* | armv6* | armv7* )
    ARCH_DIR="arm"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__ARM__"
    JAVA_ARCH="arm"

    case "$host" in
	*-hardfloat-*-gnueabi | *-gnueabihf)
		AM_CPPFLAGS="$AM_CPPFLAGS -D__ARMHF__"
		;;
	esac

	case "$host_cpu" in
	armv5* | armv6* | armv7* )
		;;
	*)
		AC_DEFINE(ARM_NO_THUMB_IW, 1, [Old ARM architecture without good Thumb interworking])
		;;
	esac
    ;;

hppa2.0 )
    ARCH_DIR="parisc"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__PARISC__"
    JAVA_ARCH="hppa"
    ;;

i386 | i486 | i586 | i686 )
    ARCH_DIR="i386"
    AM_CFLAGS="$AM_CFLAGS -m32"
    AM_CXXFLAGS="$AM_CXXFLAGS -m32"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__I386__"
    JAVA_ARCH="i386"
    ;;

mips )
    ARCH_DIR="mips"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__MIPS__"
    JAVA_ARCH="mips"
    ;;

mipsel )
    ARCH_DIR="mips"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__MIPS__"
    JAVA_ARCH="mipsel"
    ;;

powerpc )
    ARCH_DIR="powerpc"
    AM_CFLAGS="$AM_CFLAGS -m32"
    AM_CXXFLAGS="$AM_CXXFLAGS -m32"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__POWERPC__"
    JAVA_ARCH="ppc"
    ;;

powerpc64 )
    ARCH_DIR="powerpc64"
    AM_CFLAGS="$AM_CFLAGS -m64"
    AM_CXXFLAGS="$AM_CXXFLAGS -m64"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__POWERPC64__"
    JAVA_ARCH="ppc64"
    ;;

s390 )
    ARCH_DIR="s390"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__S390__"
    JAVA_ARCH="s390"
    ;;

sparc | sparc64 )
    ARCH_DIR="sparc64"
    AM_CFLAGS="$AM_CFLAGS -mcpu=v9 -m64"
    AM_CXXFLAGS="$AM_CXXFLAGS -mcpu=v9 -m64"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__SPARC_64__"
    dnl Is this correct for 64-bits?
    JAVA_ARCH="sparc"
    ;;

x86_64 )
    ARCH_DIR="x86_64"
    AM_CFLAGS="$AM_CFLAGS -m64"
    AM_CXXFLAGS="$AM_CXXFLAGS -m64"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__X86_64__"
    JAVA_ARCH="amd64"
    ;;

xdspcore )
    ARCH_DIR="xdspcore"
    ENABLE_STATICVM="yes"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__XDSPCORE__"
    USE_SCHEDULER="1"
    ;;

* )
    AC_MSG_ERROR([$host_cpu systems are currently not supported.])
    ;;
esac

dnl Host operating system type.
OS_DYNLIB=.so
case "$host_os" in
*cygwin* )
    OS_DIR="cygwin"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__WINDOWS__ -D__CYGWIN__ -D__STDC_FORMAT_MACROS"
    ;;

*darwin* )
    OS_DIR="darwin"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__DARWIN__ -D_XOPEN_SOURCE=600 -D_DARWIN_C_SOURCE -D__STDC_FORMAT_MACROS"
    OS_DYNLIB=.dynlib
    ;;

dnl Must be BEFORE *freebsd*.
*kfreebsd*-gnu)
    OS_DIR="freebsd"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__FREEBSD__ -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED -D_BSD_SOURCE -D__STDC_FORMAT_MACROS"
    ;;

*freebsd* )
    OS_DIR="freebsd"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__FREEBSD__ -D_XOPEN_SOURCE_EXTENDED -D__STDC_FORMAT_MACROS"
    ;;

*irix* )
    OS_DIR="irix"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__IRIX__ -D__STDC_FORMAT_MACROS"
    ;;

dnl Must be BEFORE *linux*.
*uclinux | *elf )
    OS_DIR="uclinux"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__LINUX__ -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED -D_BSD_SOURCE -D__STDC_FORMAT_MACROS"
    ;;

*linux* | *Linux* )
    OS_DIR="linux"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__LINUX__ -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED -D_BSD_SOURCE -D__STDC_FORMAT_MACROS"
    ;;

*netbsd* )
    OS_DIR="netbsd"
    AM_CPPFLAGS="$AM_CPPFLAGS -D__NETBSD__ -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED -D_BSD_SOURCE -D__STDC_FORMAT_MACROS"
    ;;

*solaris* )
    OS_DIR="solaris"
    dnl On solaris the architecture defines 'sparc', 'sun', 'unix' are
    dnl not set when cpp is called with -std=c99, therefore we add them
    dnl here.
    AM_CPPFLAGS="$AM_CPPFLAGS -D__SOLARIS__ -D_POSIX_C_SOURCE=200112L -D__EXTENSIONS__ -D_BSD_SOURCE -D__STDC_FORMAT_MACROS"
    dnl TODO check if still needed
    AM_CFLAGS="$AM_CFLAGS -D_XOPEN_SOURCE=600"
    AM_CXXFLAGS="$AM_CXXFLAGS -D_XOPEN_SOURCE=500"
    ;;

* )
    AC_MSG_ERROR([$host_os operating systems are currently not supported.])
    ;;
esac


dnl XXX: flag `-g` is not influenced by this arg!
dnl (configure with CFLAGS, CXXFLAGS to disable `-g`)
dnl AC_MSG_CHECKING(whether to disable optimizations and build with -g -O0)
AC_MSG_CHECKING(whether to disable optimizations and build with -O0)
AC_ARG_ENABLE([optimizations],
              [AS_HELP_STRING(--disable-optimizations,build with -O0 [[default=no]])],
              [case "${enableval}" in
                   no) disable_optimizations=yes;;
                   *) disable_optimizations=no;;
               esac],
              [disable_optimizations=no])
AC_MSG_RESULT(${disable_optimizations})

dnl save initial CFLAGS/CXXFLAGS before AC_PROG_CC/AC_PROC_CXX
init_cflags="$CFLAGS"
init_cxxflags="$CXXFLAGS"

dnl Set optimization and debugging for all architectures and systems.
if test x"$CFLAGS" = "x"; then
    AM_CFLAGS="$AM_CFLAGS -g"
fi
if test x"${disable_optimizations}" = "xyes"; then
    AM_CFLAGS="$AM_CFLAGS -O0"
else
    AM_CFLAGS="$AM_CFLAGS -O2"
fi
AM_CFLAGS="$AM_CFLAGS -fno-strict-aliasing"

if test x"$CXXFLAGS" = "x"; then
    AM_CXXFLAGS="$AM_CXXFLAGS -g"
fi
if test x"${disable_optimizations}" = "xyes"; then
    AM_CXXFLAGS="$AM_CXXFLAGS -O0"
else
    AM_CXXFLAGS="$AM_CXXFLAGS -O2"
fi
AM_CXXFLAGS="$AM_CXXFLAGS -fno-strict-aliasing"

dnl Check for compiler.
AC_PROG_CC
AC_PROG_CXX

dnl Check if compiler is usable
AC_CHECK_COMPILER

dnl discard CFLAGS/CXXFLAGS set by AC_PROG_CC/AC_PROG_CXX
CFLAGS="$init_cflags"
CXXFLAGS="$init_cxxflags"

dnl Check compiler vendor.
AX_COMPILER_VENDOR

dnl Check which C/C++ language revision to use
AC_CHECK_WITH_C_VERSION
AC_CHECK_WITH_CXX_VERSION

dnl Set compiler flags depending on the compiler vendor.
case "$ax_cv_c_compiler_vendor" in
gnu )
    AM_CFLAGS="-std=${C_LANGUAGE_VERSION} -pedantic -Wall -Wno-long-long $AM_CFLAGS"
    AM_CXXFLAGS="-std=${CXX_LANGUAGE_VERSION} -pedantic -Wall -Wno-long-long $AM_CXXFLAGS"
    ;;

sgi )
    AM_CFLAGS="-c99 $AM_CFLAGS"
    ;;

sun )
    AM_CFLAGS="-xc99 $AM_CFLAGS"
    ;;

* )
    AC_MSG_ERROR([$ax_cv_c_compiler_vendor compilers are currently not supported.])
    ;;
esac

dnl Temporary set the CFLAGS for configure tests (e.g. inline keyword)
dnl we set it properly at the end of this file.
dnl CFLAGS="$AM_CFLAGS"
dnl CXXFLAGS="$AM_CXXFLAGS"

dnl define and substitute some architecture specific variables
AC_DEFINE_UNQUOTED([ARCH_DIR], "${ARCH_DIR}", [architecture directory])
AC_DEFINE_UNQUOTED([JAVA_ARCH], "${JAVA_ARCH}", [Java architecture name])
AC_SUBST(ARCH_DIR)
AC_SUBST(JAVA_ARCH)
AC_SUBST(OS_DIR)
AC_SUBST(OS_DYNLIB)
AC_SUBST(USE_SCHEDULER)

dnl Checks for programs.
AM_PROG_AS
AC_PROG_LD
AC_PROG_LD_GNU
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P

AM_CONDITIONAL([WITH_GNU_LD], [test x"$with_gnu_ld" = "xyes"])

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC

dnl Crucial header files.
AC_CHECK_HEADERS([stdbool.h],, [AC_MSG_ERROR(cannot find stdbool.h)])
AC_CHECK_HEADERS([stdint.h],, [AC_MSG_ERROR(cannot find stdint.h)])

dnl keep them alpha-sorted!
AC_CHECK_HEADERS([assert.h])
AC_CHECK_HEADERS([errno.h])
AC_CHECK_HEADERS([execinfo.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([libgen.h])
AC_CHECK_HEADERS([locale.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([signal.h])
AC_CHECK_HEADERS([stdarg.h])
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([ucontext.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([mach/mach.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/loadavg.h])
AC_CHECK_HEADERS([sys/mman.h])
AC_CHECK_HEADERS([sys/resource.h])
AC_CHECK_HEADERS([sys/select.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/utsname.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_ATTRIBUTE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl Checks for libraries (NOTE: Should be done before function checks,
dnl as some functions may be in libraries we check for).
case "${OS_DIR}" in
    solaris )
         AC_CHECK_LIB([socket], [gethostname],, [AC_MSG_ERROR(cannot find libsocket)])
         ;;
esac

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_TYPE_SIGNAL
AC_FUNC_MEMCMP
AC_FUNC_MMAP

dnl keep them alpha-sorted!
AC_CHECK_FUNCS([abort])
AC_CHECK_FUNCS([accept])
AC_CHECK_FUNCS([access])
AC_CHECK_FUNCS([atoi])
AC_CHECK_FUNCS([atol])
AC_CHECK_FUNCS([backtrace])
AC_CHECK_FUNCS([backtrace_symbols])
AC_CHECK_FUNCS([calloc])
AC_CHECK_FUNCS([close])
AC_CHECK_FUNCS([confstr])
AC_CHECK_FUNCS([connect])
AC_CHECK_FUNCS([dirname])
AC_CHECK_FUNCS([fclose])
AC_CHECK_FUNCS([fflush])
AC_CHECK_FUNCS([fopen])
AC_CHECK_FUNCS([fprintf])
AC_CHECK_FUNCS([vfprintf])
AC_CHECK_FUNCS([fread])
AC_CHECK_FUNCS([free])
AC_CHECK_FUNCS([fstat])
AC_CHECK_FUNCS([fsync])
AC_CHECK_FUNCS([ftruncate])
AC_CHECK_FUNCS([getcwd])
AC_CHECK_FUNCS([getenv])
AC_CHECK_FUNCS([gethostbyname])
AC_CHECK_FUNCS([gethostname])
AC_CHECK_FUNCS([getloadavg])
AC_CHECK_FUNCS([getpid])
AC_CHECK_FUNCS([getrusage])
AC_CHECK_FUNCS([getsockname])
AC_CHECK_FUNCS([getsockopt])
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([htons])
AC_CHECK_FUNCS([ioctl])
AC_CHECK_FUNCS([isnan])
AC_CHECK_FUNCS([listen])
AC_CHECK_FUNCS([localtime])
AC_CHECK_FUNCS([localtime_r])
AC_CHECK_FUNCS([lseek])
AC_CHECK_FUNCS([malloc])
AC_CHECK_FUNCS([memcpy])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([mmap])
AC_CHECK_FUNCS([mprotect])
AC_CHECK_FUNCS([open])
AC_CHECK_FUNCS([printf])
AC_CHECK_FUNCS([read])
AC_CHECK_FUNCS([readlink])
AC_CHECK_FUNCS([realloc])
AC_CHECK_FUNCS([recv])
AC_CHECK_FUNCS([scandir])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([send])
AC_CHECK_FUNCS([setlocale])
AC_CHECK_FUNCS([setsockopt])
AC_CHECK_FUNCS([shutdown])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([stat])
AC_CHECK_FUNCS([str2sig])
AC_CHECK_FUNCS([strcat])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strcpy])
AC_CHECK_FUNCS([strcmp])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strlen])
AC_CHECK_FUNCS([strncmp])
AC_CHECK_FUNCS([strstr])
AC_CHECK_FUNCS([time])
AC_CHECK_FUNCS([write])

AM_LC_MESSAGES

dnl Checks for libraries.

dnl Checking sizeof void *
AC_CHECK_SIZEOF(void *)
AC_C_BIGENDIAN


dnl define install prefix
if test "x$prefix" = "xNONE"; then
    CACAO_PREFIX=$ac_default_prefix
else
    CACAO_PREFIX=$prefix
fi
AC_DEFINE_UNQUOTED([CACAO_PREFIX], "${CACAO_PREFIX}", [installation prefix])
AC_SUBST(CACAO_PREFIX)

dnl define libdir prefix
if test "x$libdir" = "xNONE"; then
    CACAO_LIBDIR=$ac_default_libdir
else
    CACAO_LIBDIR=$libdir
fi
dnl expand CACAO_LIBDIR to something that is usable in C code
AS_AC_EXPAND([CACAO_LIBDIR], ${CACAO_LIBDIR})
AC_DEFINE_UNQUOTED([CACAO_LIBDIR], "${CACAO_LIBDIR}", [library installation prefix])
AC_SUBST(CACAO_LIBDIR)


AC_CHECK_ENABLE_DL
AC_CHECK_ENABLE_JAVA

AC_CHECK_ENABLE_JIT
AC_CHECK_ENABLE_INTRP

if test x"${ENABLE_JIT}" = "xno" -a x"${ENABLE_INTRP}" = "xno"; then
    AC_ERROR(You have to enable at least the JIT or the interpreter.)
fi

AC_CHECK_SOFTFLOAT
AC_CHECK_SOFT_FLOAT_CMP
AC_CHECK_SOFT_DOUBLE_CMP

AC_CHECK_ENABLE_GC
AC_CHECK_ENABLE_DEBUG
AC_CHECK_ENABLE_WERROR
if test x"${enable_werror}" == "xyes"; then
	AM_CXXFLAGS="$AM_CXXFLAGS -Werror"
	AM_CFLAGS="$AM_CFLAGS -Werror"
fi
AC_CHECK_ENABLE_DUMP
AC_CHECK_ENABLE_MEMCHECK
AC_CHECK_ENABLE_HANDLES
AC_CHECK_ENABLE_LOGGING
AC_CHECK_ENABLE_DISASSEMBLER
AC_CHECK_ENABLE_STATISTICS
AC_CHECK_ENABLE_VERIFIER
AC_CHECK_ENABLE_RT_TIMING
AC_CHECK_ENABLE_CYCLE_STATS
AC_CHECK_ENABLE_JVMTI
AC_CHECK_ENABLE_THREADS
if test x"${ENABLE_THREADS}" != "xno"; then
    AC_CHECK_USE___THREAD
fi
AC_CHECK_ENABLE_IFCONV
AC_CHECK_ENABLE_INLINING
AC_CHECK_ENABLE_INLINING_DEBUG
AC_CHECK_ENABLE_REPLACEMENT

dnl check for loop optimization
AC_MSG_CHECKING(whether loop optimization should be supported)
AC_ARG_ENABLE([loop],
              [AS_HELP_STRING(--enable-loop,enable loop optimization [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_LOOP=yes;;
                   *) ENABLE_LOOP=no;;
               esac],
              [ENABLE_LOOP=no])
AC_MSG_RESULT(${ENABLE_LOOP})
AM_CONDITIONAL([ENABLE_LOOP], test x"${ENABLE_LOOP}" = "xyes")

if test x"${ENABLE_LOOP}" = "xyes"; then
    AC_DEFINE([ENABLE_LOOP], 1, [use loop optimization])
fi


dnl check if linear scan register allocator(lsra) with SSA should be used
AC_MSG_CHECKING(whether lsra with ssa should be supported)
AC_ARG_ENABLE([ssa],
              [AS_HELP_STRING(--disable-ssa,disable ssa [[default=disabled]])],
              [case "${enableval}" in
                   no) ENABLE_SSA=no;;
                   *) ENABLE_SSA=yes;;
               esac],
              [ENABLE_SSA=no])
AC_MSG_RESULT(${ENABLE_SSA})
AM_CONDITIONAL([ENABLE_SSA], test x"${ENABLE_SSA}" = "xyes")
AM_CONDITIONAL([ENABLE_ESCAPE], test x"${ENABLE_SSA}" = "xyes")
AM_CONDITIONAL([ENABLE_ESCAPE_CHECK], test x"${ENABLE_SSA}" = "xyes")
AM_CONDITIONAL([ENABLE_TLH], test x"${ENABLE_SSA}" = "xyes")

if test x"${ENABLE_SSA}" = "xyes"; then
    AC_DEFINE([ENABLE_SSA], 1, [enable lsra with ssa])
    AC_DEFINE([ENABLE_ESCAPE], 1, [enable escape analysis with ssa])
    AC_DEFINE([ENABLE_ESCAPE_CHECK], 1, [enable generating code to validate escape analysis results])
    AC_DEFINE([ENABLE_TLH], 1, [enable thread local heap])
    ENABLE_LSRA="no"
fi


dnl check if linear scan register allocator(lsra) should be used
AC_MSG_CHECKING(whether lsra should be supported)
AC_ARG_ENABLE([lsra],
              [AS_HELP_STRING(--enable-lsra,enable linear scan register allocator [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_LSRA=yes;;
                   *) ENABLE_LSRA=no;;
               esac],
              [ENABLE_LSRA=no])
   
if test x"${ENABLE_LSRA}" = "xyes"; then
    if test x"${ENABLE_SSA}" = "xyes"; then
        ENABLE_LSRA="no"
    fi
fi
AC_MSG_RESULT(${ENABLE_LSRA})
AM_CONDITIONAL([ENABLE_LSRA], test x"${ENABLE_LSRA}" = "xyes")
 
if test x"${ENABLE_LSRA}" = "xyes"; then
    AC_DEFINE([ENABLE_LSRA], 1, [enable lsra])
fi

dnl check if profiling should be supported
AC_MSG_CHECKING(whether profiling should be supported)
AC_ARG_ENABLE([profiling],
              [AS_HELP_STRING(--enable-profiling,enable profiling [[default=no]])],
              [case "${enableval}" in
                   yes) ENABLE_PROFILING=yes;;
                   *) ENABLE_PROFILING=no;;
               esac],
              [ENABLE_PROFILING=no])
AC_MSG_RESULT(${ENABLE_PROFILING})
AM_CONDITIONAL([ENABLE_PROFILING], test x"${ENABLE_PROFILING}" = "xyes")
   
if test x"${ENABLE_PROFILING}" = "xyes"; then
    AC_DEFINE([ENABLE_PROFILING], 1, [enable profiling])
fi


dnl check for instruction scheduler
AC_ARG_ENABLE([scheduler], [AS_HELP_STRING(--enable-scheduler,enable instruction scheduler [[default=no]])])

AC_MSG_CHECKING(whether instruction scheduler should be supported)
if test x"${USE_SCHEDULER}" = "x1" -o x"$enable_scheduler" = "xyes"; then
	case "${ARCH_DIR}" in
		xdspcore )
			;;
		* )
			AC_MSG_ERROR(instruction scheduler is not available for ${ARCH_DIR} code generator)
			;;
	esac

	AC_MSG_RESULT(yes)
	AC_DEFINE([USE_SCHEDULER], 1, [use scheduler])
	AM_CONDITIONAL([USE_SCHEDULER], [true])
else
	AC_MSG_RESULT(no)
	AM_CONDITIONAL([USE_SCHEDULER], [false])
fi

AC_CHECK_ENABLE_OPAGENT

AC_CHECK_ENABLE_ZLIB

dnl check for stuff to be built
AC_CHECK_ENABLE_JNI
AC_CHECK_ENABLE_LIBJVM

dnl ATTENTION: These check must be after setting
dnl enable_shared/enable_static (happens in AC_CHECK_ENABLE_LIBJVM),
dnl because libtool uses them to build the correct libraries.
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_CHECK_ENABLE_STATICVM
AC_CHECK_ENABLE_ANNOTATIONS
AC_CHECK_ENABLE_ASSERTION

dnl This one depends on AC_CHECK_ENABLE_ZLIB.
AC_CHECK_WITH_VM_ZIP
AC_CHECK_HGREV

AC_CHECK_WITH_JAVA_RUNTIME_LIBRARY

dnl Now we check for jre-layout so we can skip some checks that are
dnl not required.
AC_CHECK_ENABLE_JRE_LAYOUT

AC_CHECK_WITH_JAVA_RUNTIME_LIBRARY_PREFIX
AC_CHECK_WITH_JAVA_RUNTIME_LIBRARY_CLASSES
AC_CHECK_WITH_BUILD_JAVA_RUNTIME_LIBRARY_CLASSES

if test x"${ENABLE_JRE_LAYOUT}" = "xno"; then
   AC_CHECK_WITH_JAVA_RUNTIME_LIBRARY_LIBDIR
fi

dnl The check for jni_md.h must be before jni.h.
AC_CHECK_WITH_JNI_MD_H
AC_CHECK_WITH_JNI_H

dnl The jvmti.h header is only required if JVMTI is enabled.
if test x"${ENABLE_JVMTI}" = "xyes"; then
    AC_CHECK_WITH_JVMTI_H
fi

dnl JVM, HPI, and JMM is only required for OpenJDK.
case "${WITH_JAVA_RUNTIME_LIBRARY}" in
    openjdk)
        AC_CHECK_WITH_JVM_MD_H
        AC_CHECK_WITH_JVM_H
        AC_CHECK_WITH_HPI_MD_H
        AC_CHECK_WITH_HPI_H
        AC_CHECK_WITH_JMM_H
        ;;
    openjdk7)
        AC_CHECK_WITH_JVM_MD_H
        AC_CHECK_WITH_JVM_H
        AC_CHECK_WITH_JMM_H
        ;;
    *)
        ;;
esac

dnl check for some programs we need

case "${WITH_JAVA_RUNTIME_LIBRARY}" in
    cldc1.1 | gnuclasspath)
        AC_PROG_JAVAC
        ;;
    openjdk | openjdk7)
        ;;
    *)
        AC_MSG_ERROR(unknown classpath configuration ${WITH_JAVA_RUNTIME_LIBRARY})
        ;;
esac

AC_PROG_JAR
AC_CHECK_WITH_JASMIN_JAR
AC_CHECK_WITH_JUNIT_JAR

AC_CHECK_DISABLE_TEST_DEPENDENCY_CHECKS

if test x"${disable_test_dependency_checks}" = "xno"; then
	AC_CHECK_JASMIN_WORKS
	AC_CHECK_JUNIT_WORKS
fi

GTEST_LIB_CHECK

AC_CHECK_HEADERS(
	[regex.h], 
	[
		dnl If debug build and regex.h available, enable debug filter
		if test x"${NDEBUG}" = "xno"; then
			AC_DEFINE(ENABLE_DEBUG_FILTER, 1, [debug filter])
		fi
	]
)

AC_CHECK_FUTURE

AC_CHECK_HAVE_BUILTIN_STATIC_ASSERT

AC_CHECK_ENABLE_EXPENSIVE_ASSERT

AC_CHECK_ENABLE_GCC_PR40134

AC_CHECK_ENABLE_PIC_ASM

dnl Define version numbers.
AC_VERSION_DETAIL
AC_VERSION_CONFIG

dnl Finally pass flags to Makefiles.
dnl CFLAGS="$AM_CFLAGS"
dnl CXXFLAGS="$AM_CXXFLAGS"

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_CPPFLAGS)


AC_CONFIG_FILES([Makefile]
		[contrib/Makefile]
		[contrib/setenvinstalled]
		[contrib/setenvsource]
		[contrib/warnings.ignore]
		[doc/Makefile]
		[doc/abc_optimization/Makefile]
		[doc/doxygen/Makefile]
		[doc/doxygen/Doxyfile]
		[doc/handbook/Makefile]
		[doc/intern_tables/Makefile]
		[man/Makefile]
		[src/Makefile]
		[src/cacao/Makefile]
		[src/classes/Makefile]
		[src/fdlibm/Makefile]
		[src/future/Makefile]
		[src/mm/Makefile]
		[src/mm/cacao-gc/Makefile]
		[src/native/Makefile]
		[src/native/jvmti/Makefile]
		[src/native/vm/Makefile]
		[src/native/vm/cldc1.1/Makefile]
		[src/native/vm/gnuclasspath/Makefile]
		[src/native/vm/openjdk/Makefile]
		[src/scripts/Makefile]
		[src/scripts/java]
		[src/threads/Makefile]
		[src/threads/none/Makefile]
		[src/threads/posix/Makefile]
		[src/toolbox/Makefile]
		[src/vm/Makefile]
		[src/vm/jit/Makefile]
		[src/vm/jit/allocator/Makefile]
		[src/vm/jit/alpha/Makefile]
		[src/vm/jit/alpha/freebsd/Makefile]
		[src/vm/jit/alpha/linux/Makefile]
		[src/vm/jit/arm/Makefile]
		[src/vm/jit/arm/linux/Makefile]
		[src/vm/jit/i386/Makefile]
		[src/vm/jit/i386/cygwin/Makefile]
		[src/vm/jit/i386/darwin/Makefile]
		[src/vm/jit/i386/freebsd/Makefile]
		[src/vm/jit/i386/linux/Makefile]
		[src/vm/jit/i386/solaris/Makefile]
		[src/vm/jit/intrp/Makefile]
		[src/vm/jit/inline/Makefile]
		[src/vm/jit/ir/Makefile]
		[src/vm/jit/loop/Makefile]
		[src/vm/jit/mips/Makefile]
		[src/vm/jit/mips/irix/Makefile]
		[src/vm/jit/mips/linux/Makefile]
		[src/vm/jit/mips/uclinux/Makefile]
		[src/vm/jit/optimizing/Makefile]
		[src/vm/jit/parisc/Makefile]
		[src/vm/jit/parisc/linux/Makefile]
		[src/vm/jit/powerpc/Makefile]
		[src/vm/jit/powerpc/darwin/Makefile]
		[src/vm/jit/powerpc/linux/Makefile]
		[src/vm/jit/powerpc/netbsd/Makefile]
		[src/vm/jit/powerpc64/Makefile]
		[src/vm/jit/powerpc64/linux/Makefile]
		[src/vm/jit/s390/Makefile]
		[src/vm/jit/schedule/Makefile]
		[src/vm/jit/sparc64/Makefile]
		[src/vm/jit/sparc64/linux/Makefile]
		[src/vm/jit/sparc64/solaris/Makefile]
		[src/vm/jit/verify/Makefile]
		[src/vm/jit/verify/typecheck-stackbased-gen.inc]
		[src/vm/jit/verify/typecheck-variablesbased-gen.inc]
		[src/vm/jit/verify/typecheck-typeinferer-gen.inc]
		[src/vm/jit/x86_64/Makefile]
		[src/vm/jit/x86_64/darwin/Makefile]
		[src/vm/jit/x86_64/freebsd/Makefile]
		[src/vm/jit/x86_64/linux/Makefile]
		[src/vm/jit/x86_64/solaris/Makefile]
		[tests/Makefile]
		[tests/gtest/Makefile]
		[tests/jsr292/Makefile]
		[tests/regression/Makefile]
		[tests/regression/assertion/Makefile]
		[tests/regression/base/Makefile]
		[tests/regression/bugzilla/Makefile]
		[tests/regression/jasmin/Makefile]
		[tests/regression/native/Makefile]
		[tests/regression/resolving/Makefile]
		[tests/regression/resolving/classes1/Makefile]
		[tests/regression/resolving/classes2/Makefile]
		[tests/regression/resolving/classes3/Makefile]
)


dnl We only build the shared library objects but we use it as
dnl noinst_LTLIBRARIES.
ac_configure_args="$ac_configure_args --disable-static"
AC_CONFIG_SUBDIRS(src/mm/boehm-gc)


AC_OUTPUT


## Local variables:
## mode: m4
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
## vim:noexpandtab:sw=4:ts=4:
