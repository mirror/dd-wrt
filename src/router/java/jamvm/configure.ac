##
## Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
## 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
##
## This file is part of JamVM.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2,
## or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
##

dnl Process this file with autoconf to produce a configure script.

AC_INIT(jamvm, 2.0.0)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/jam.c)
AC_CONFIG_HEADERS([src/config.h])
AC_PREFIX_DEFAULT(/usr/local/jamvm)
AM_INIT_AUTOMAKE()

AC_CANONICAL_HOST

case "$host" in
i[[3456]]86-*-linux*) host_cpu=i386 host_os=linux ;;
i[[3456]]86-*-kfreebsd*) host_cpu=i386 host_os=linux ;;
i[[3456]]86-*-darwin*) host_cpu=i386 host_os=darwin ;;
arm*-*-darwin*) host_cpu=arm host_os=darwin libdl_needed=no ;;
i386-*-openbsd*) host_os=bsd libdl_needed=no ;;
i386-*-freebsd*) host_os=bsd libdl_needed=no ;;
i386-*-solaris*) host_cpu=x86 host_os=solaris ;;
x86_64-*-linux*) host_os=linux ;;
hppa*-*-linux*) host_cpu=parisc host_os=linux ;;
mips*-*-linux*) host_cpu=mips host_os=linux ;;
x86_64-*-openbsd*) host_os=bsd libdl_needed=no ;;
x86_64-*-freebsd*) host_os=bsd libdl_needed=no ;;
arm*-*-linux*) host_cpu=arm host_os=linux interp_cflags=-marm ;;
arm*-*-openbsd*) host_cpu=arm host_os=bsd libdl_needed=no ;;
arm*-*-freebsd*) host_cpu=arm host_os=bsd libdl_needed=no ;;
powerpc*-*-linux*) host_cpu=powerpc host_os=linux ;;
powerpc*-*-openbsd*) host_cpu=powerpc host_os=bsd libdl_needed=no ;;
powerpc*-*-freebsd*) host_cpu=powerpc host_os=bsd libdl_needed=no ;;
powerpc-*-darwin*) host_os=darwin ;;
sparc*-*-openbsd*) host_cpu=sparc host_os=bsd libdl_needed=no ;;
sparc*-*-freebsd*) host_cpu=sparc host_os=bsd libdl_needed=no ;;
*) AC_MSG_ERROR($host not supported) ;;
esac

arch=$host_cpu
os=$host_os
AC_SUBST(arch)
AC_SUBST(os)

AC_ARG_ENABLE(tracegc,
    [AS_HELP_STRING(--enable-tracegc,add gc tracing (for debugging))],
    [if test "$enableval" != no; then
        AC_DEFINE([TRACEGC],1,[defined if gc tracing enabled for debugging])
    fi],)

AC_ARG_ENABLE(tracealloc,
    [AS_HELP_STRING(--enable-tracealloc,add object allocation tracing (for debugging))],
    [if test "$enableval" != no; then
        AC_DEFINE([TRACEALLOC],1,[defined if object allocation tracing enabled for debugging])
    fi],)

AC_ARG_ENABLE(tracefnlz,
    [AS_HELP_STRING(--enable-tracefnlz,add object finalisation tracing (for debugging))],
    [if test "$enableval" != no; then
        AC_DEFINE([TRACEFNLZ],1,[defined if object finalisation tracing enabled for debugging])
    fi],)

AC_ARG_ENABLE(tracedll,
    [AS_HELP_STRING(--enable-tracedll,add library loading/lookup tracing (for debugging))],
    [if test "$enableval" != no; then
        AC_DEFINE([TRACEDLL],1,[defined if library loading/unloading tracing enabled for debugging])
    fi],)

AC_ARG_ENABLE(tracelock,
    [AS_HELP_STRING(--enable-tracelock,add object locking tracing (for debugging))],
    [if test "$enableval" != no; then
        AC_DEFINE([TRACELOCK],1,[defined if object locking tracing enabled for debugging])
    fi],)

AC_ARG_ENABLE(tracethread,
    [AS_HELP_STRING(--enable-tracethread,add thread creation tracing (for debugging))],
    [if test "$enableval" != no; then
        AC_DEFINE([TRACETHREAD],1,[defined if thread creation tracing enabled for debugging])
    fi],)

AC_ARG_ENABLE(tracecompact,
    [AS_HELP_STRING(--enable-tracecompact,add GC compaction phase tracing (for debugging))],
    [if test "$enableval" != no; then
        AC_DEFINE([TRACECOMPACT],1,[defined if GC compaction phase tracing enabled for debugging])
    fi],)

AC_ARG_ENABLE(tracedirect,
    [AS_HELP_STRING(--enable-tracedirect,add interpreter direct-mode tracing (for debugging))],
    [if test "$enableval" != no; then
        AC_DEFINE([TRACEDIRECT],1,[defined if interpreter direct-mode tracing enabled for debugging])
    fi],)

AC_ARG_ENABLE(traceinlining,
    [AS_HELP_STRING(--enable-traceinlining,add interpreter inlining tracing (for debugging))],
    [if test "$enableval" != no; then
        AC_DEFINE([TRACEINLINING],1,[defined if interpreter inlining tracing enabled for debugging])
    fi],)

AC_ARG_ENABLE(trace,
    [AS_HELP_STRING(--enable-trace,add all tracing (for debugging))],
    [if test "$enableval" != no; then
        AC_DEFINE([TRACEGC],1,[defined if gc tracing enabled for debugging])
        AC_DEFINE([TRACEALLOC],1,[defined if object allocation tracing enabled for debugging])
        AC_DEFINE([TRACEFNLZ],1,[defined if object finalisation tracing enabled for debugging])
        AC_DEFINE([TRACEDLL],1,[defined if library loading/unloading tracing enabled for debugging])
        AC_DEFINE([TRACELOCK],1,[defined if object locking tracing enabled for debugging])
        AC_DEFINE([TRACETHREAD],1,[defined if thread creation tracing enabled for debugging])
        AC_DEFINE([TRACECOMPACT],1,[defined if GC compaction phase tracing enabled for debugging])
        AC_DEFINE([TRACEDIRECT],1,[defined if interpreter direct-mode tracing enabled for debugging])
        AC_DEFINE([TRACEINLINING],1,[defined if interpreter inlining tracing enabled for debugging])
    fi],)

AC_ARG_ENABLE(int-threading,
    [AS_HELP_STRING(--enable-int-threading,enable threaded version of the interpreter (default enabled))],,)

AC_ARG_ENABLE(int-direct,
    [AS_HELP_STRING(--enable-int-direct,enable direct threaded version of the interpreter (default enabled))],,)

AC_ARG_ENABLE(int-caching,
    [AS_HELP_STRING(--enable-int-caching,enable stack-caching version of the interpreter
                   (disabled by default on x86-64, enabled otherwise))],,
    [if test "$host_cpu" = x86_64; then
         enable_int_caching=no
     fi])

AC_ARG_ENABLE(int-prefetch,
    [AS_HELP_STRING(--enable-int-prefetch,enable prefetching version of the interpreter
                   (by default enabled on powerpc, disabled otherwise))],,
    [if test "$host_cpu" != powerpc; then
         enable_int_prefetch=no
     fi])

AC_ARG_ENABLE(runtime-reloc-checks,
    [AS_HELP_STRING(--enable-runtime-reloc-checks,compute relocatability at runtime
                   (inlining interpreter, enabled by default if cross-compiling))],,
    [enable_runtime_reloc_checks="$cross_compiling"])

AC_ARG_ENABLE(int-inlining,
    [AS_HELP_STRING(--enable-int-inlining,enable inline threaded version of the interpreter
                   (by default enabled on x86_64, i386, powerpc, mips and arm, 
                    disabled otherwise))],,
    [if test "$host_cpu" = x86_64 -o "$host_cpu" = i386 -o "$host_cpu" = x86 -o \
             "$host_cpu" = powerpc -o "$host_cpu" = arm -o "$host_cpu" = mips; then
         enable_int_inlining=yes
       else
         enable_int_inlining=no
     fi])

if test "$enable_int_threading" != no; then
    AC_DEFINE([THREADED],1,[interpreter threaded])

    if test "$enable_int_caching" != no; then
        AC_DEFINE([USE_CACHE],1,[interpreter uses caching])
    fi

    if test "$enable_int_direct" != no; then
        AC_DEFINE([DIRECT],1,[interpreter direct])

        if test "$enable_int_prefetch" != no; then
            AC_DEFINE([PREFETCH],1,[interpreter uses prefetching])
        fi

        if test "$enable_int_inlining" != no; then
            AC_DEFINE([INLINING],1,[interpreter inlining])
            interp_cflags="$interp_cflags -fno-reorder-blocks"

            if test "$enable_runtime_reloc_checks" != no; then
                AC_DEFINE([RUNTIME_RELOC_CHECKS],1,[compute relocatability at runtime])
            else
                compile_time_reloc_checks=yes
            fi
        fi
    fi
fi

AC_SUBST(interp_cflags)
AM_CONDITIONAL(COMPILE_TIME_RELOC_CHECKS, test "$compile_time_reloc_checks" = yes)

AC_ARG_ENABLE(ffi,
    [AS_HELP_STRING(--enable-ffi,use libffi to call native methods
                                 (enabled by default on parisc and sparc))],,
    [if test "$host_cpu" != parisc -a "$host_cpu" != sparc; then
        enable_ffi=no
     fi])

AC_ARG_ENABLE(md-stubs,
    [AS_HELP_STRING(--enable-md-stubs,use machine-dependent JNI stubs rather than generic 
                                 (enabled by default on i386))],,
    [if test "$host_cpu" != i386; then
        enable_md_stubs=no
     fi])

AC_ARG_ENABLE(profile-stubs,
    [AS_HELP_STRING(--enable-profile-stubs,JNI stubs support profiling)],,)

AC_ARG_WITH(java-runtime-library,
    [AS_HELP_STRING(--with-java-runtime-library=[[[gnuclasspath|openjdk6|openjdk7|openjdk8]]],
                    which Java runtime library interface to use (default gnuclasspath))],
    [if test "$withval" != gnuclasspath -a "$withval" != openjdk6 -a \
             "$withval" != openjdk7 -a "$withval" != openjdk8; then
        AC_MSG_ERROR($withval Java runtime library is not supported)
     fi],
    [with_java_runtime_library=gnuclasspath])

AC_ARG_WITH(classpath-install-dir,
    [AS_HELP_STRING(--with-classpath-install-dir=<dir>,installation directory of GNU classpath
                   (default /usr/local/classpath))],,
    [with_classpath_install_dir=/usr/local/classpath])

AC_DEFINE_UNQUOTED(CLASSPATH_INSTALL_DIR, "$with_classpath_install_dir",
    [GNU Classpath installation directory (prefix)])

AC_SUBST(with_classpath_install_dir)

if test "$prefix" = "NONE"; then
    install_dir=$ac_default_prefix
else
    install_dir=$prefix
fi
AC_DEFINE_UNQUOTED(INSTALL_DIR, "$install_dir", [Installation directory (prefix)])

AC_ARG_ENABLE(zip,
    [AS_HELP_STRING(--disable-zip,turn-off zip support in the bootstrap loader)],,)

AC_ARG_ENABLE(tls,
    [AS_HELP_STRING(--enable-tls,use thread-local-storage (enabled by default if supported by compiler))],,)

dnl Initialise libtool
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

dnl Checks for programs.
AC_PROG_CC
AM_PROG_AS

AC_CHECK_PROGS(JAVAC, ecj jikes "gcj -C" javac)

dnl Checks for libraries.

dnl Prefer libthr to libpthread (for FreeBSD)
AC_CHECK_LIB(thr,pthread_self,,[
  AC_CHECK_LIB(pthread,pthread_self,,AC_MSG_ERROR(libpthread is missing))
])

AC_CHECK_LIB(rt,clock_gettime,,)

AC_CHECK_LIB(m,fmod,,AC_MSG_ERROR(libm is missing))

if test "$libdl_needed" != no; then
    AC_CHECK_LIB(dl,dlopen,,AC_MSG_ERROR(libdl is missing))
fi

if test "$enable_zip" != no; then
    AC_CHECK_LIB(z,inflate,,AC_MSG_ERROR(zlib is missing))
fi

if test "$enable_ffi" != no; then
    AC_CHECK_LIB(ffi,ffi_call,,AC_MSG_ERROR(libffi is missing))
fi

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/time.h unistd.h endian.h sys/param.h locale.h alloca.h)

if test "$enable_zip" != no; then
    AC_CHECK_HEADER(zlib.h,,AC_MSG_ERROR(zlib.h is missing))
fi

if test "$enable_ffi" != no; then
    AC_CHECK_HEADER(ffi.h,,AC_MSG_ERROR(ffi.h is missing))
fi

if test "$enable_zip" != no; then
    AC_DEFINE([USE_ZIP],1,[use zip])
    use_zip_yes=
    use_zip_no='#'
else
    use_zip_yes='#'
    use_zip_no=
fi

AC_SUBST(use_zip_yes)
AC_SUBST(use_zip_no)

if test "$enable_ffi" != no; then
    AC_DEFINE([USE_FFI],1,[use FFI])
fi

if test "$enable_md_stubs" != no; then
    AC_DEFINE([USE_MD_STUBS],1,[use machine-dependent JNI stubs])
fi

if test "$enable_profile_stubs" = yes; then
    AC_DEFINE([HAVE_PROFILE_STUBS],1,[JNI stubs support profiling])
fi

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MMAP
AC_CHECK_FUNCS(gettimeofday strtol setlocale)
AM_LC_MESSAGES

dnl This check initially determined if thread local storage is supported by
dnl trying to compile and link a program that uses TLS.  However, on several
dnl OS/C-libraries, the check succeeds but the program doesn't actually run
dnl (the check can't run the program as this won't work when cross-compiling).
dnl So we take a conservative approach, and only check OS/C-library
dnl combinations which should work (currenty Linux/glibc).

if test "$enable_tls" != no; then
    AC_MSG_CHECKING([for __thread])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <features.h>
                                     #if !defined(__linux__) || !defined(__GLIBC__)
                                         #error not supported on this OS/C library
                                     #endif]
                                   ,[static __thread int foo = 0])],
        [AC_DEFINE([HAVE_TLS],1,[__thread supported by compiler])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])
fi

dnl configure class-library support

if test "$with_java_runtime_library" != gnuclasspath; then
    ver=`echo "$with_java_runtime_library" | $SED 's/^openjdk\([[0-9]]\)$/\1/'`
    AC_DEFINE_UNQUOTED([OPENJDK_VERSION],$ver,[OpenJDK class-library])
    classlib=openjdk
else
    classlib=gnuclasspath
fi

AC_SUBST(classlib)

case "$with_java_runtime_library" in
    gnuclasspath | openjdk6 )
        AC_DEFINE([SHARED_CHAR_BUFFERS],1,
                  [java.lang.String uses shared char buffers]) ;;

    openjdk7 )
        AC_DEFINE([JSR292],1,[VM support for JSR292 (invokedynamic)]) ;;

    openjdk8 )
        AC_DEFINE([JSR292],1,[VM support for JSR292 (invokedynamic)])
        AC_DEFINE([JSR308],1,[VM support for JSR308 (type annotations)])
        AC_DEFINE([JSR335],1,[VM support for JSR335 (lambda expressions)])
        AC_DEFINE([JSR901],1,[VM support for JSR901 (method parameter reflection)]) ;;
esac

dnl split version into separate numbers
major=`echo "$VERSION" | $SED 's/^\([[0-9]][[0-9]]*\)\.[[0-9]][[0-9]]*\.[[0-9]][[0-9]]*.*$/\1/'`
minor=`echo "$VERSION" | $SED 's/^[[0-9]][[0-9]]*\.\([[0-9]][[0-9]]*\)\.[[0-9]][[0-9]]*.*$/\1/'`
micro=`echo "$VERSION" | $SED 's/^[[0-9]][[0-9]]*\.[[0-9]][[0-9]]*\.\([[0-9]][[0-9]]*\).*$/\1/'`

if test "$major" = "$VERSION"; then
    AC_MSG_ERROR(Version string is malformed)
fi

AC_DEFINE_UNQUOTED([VERSION_MAJOR], $major, [major version number])
AC_DEFINE_UNQUOTED([VERSION_MINOR], $minor, [minor version number])
AC_DEFINE_UNQUOTED([VERSION_MICRO], $micro, [micro version number])

AM_MAINTAINER_MODE

AC_CONFIG_LINKS(src/arch.h:src/arch/$arch.h
                src/classlib.h:src/classlib/$classlib/classlib.h
                src/classlib-defs.h:src/classlib/$classlib/classlib-defs.h
                src/classlib-excep.h:src/classlib/$classlib/classlib-excep.h
                src/classlib-symbol.h:src/classlib/$classlib/classlib-symbol.h)

AC_CONFIG_FILES(
    Makefile \
    src/Makefile \
    src/interp/Makefile \
    src/interp/engine/Makefile \
    src/arch/Makefile \
    src/os/Makefile \
    src/os/linux/Makefile \
    src/os/darwin/Makefile \
    src/os/bsd/Makefile \
    src/os/solaris/Makefile \
    src/os/solaris/x86/Makefile \
    src/os/linux/powerpc/Makefile \
    src/os/linux/arm/Makefile \
    src/os/linux/i386/Makefile \
    src/os/linux/x86_64/Makefile \
    src/os/linux/parisc/Makefile \
    src/os/linux/mips/Makefile \
    src/os/darwin/i386/Makefile \
    src/os/darwin/arm/Makefile \
    src/os/darwin/powerpc/Makefile \
    src/os/bsd/powerpc/Makefile \
    src/os/bsd/arm/Makefile \
    src/os/bsd/i386/Makefile \
    src/os/bsd/x86_64/Makefile \
    src/os/bsd/sparc/Makefile \
    src/classlib/Makefile \
    src/classlib/openjdk/Makefile \
    src/classlib/gnuclasspath/Makefile \
    src/classlib/gnuclasspath/lib/Makefile \
    src/classlib/gnuclasspath/lib/java/Makefile \
    src/classlib/gnuclasspath/lib/java/lang/Makefile \
    src/classlib/gnuclasspath/lib/jamvm/Makefile \
    src/classlib/gnuclasspath/lib/jamvm/java/Makefile \
    src/classlib/gnuclasspath/lib/jamvm/java/lang/Makefile \
    src/classlib/gnuclasspath/lib/java/lang/reflect/Makefile \
    src/classlib/gnuclasspath/lib/java/security/Makefile \
    src/classlib/gnuclasspath/lib/gnu/Makefile \
    src/classlib/gnuclasspath/lib/sun/reflect/annotation/Makefile \
    src/classlib/gnuclasspath/lib/sun/reflect/Makefile \
    src/classlib/gnuclasspath/lib/sun/Makefile \
    src/classlib/gnuclasspath/lib/gnu/classpath/Makefile)

AC_OUTPUT

