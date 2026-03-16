#compiler related checks, updates CFLAGS and CPPFLAGS and sets
# 'warn' with flags for warnings

dnl Checks for programs.
AC_PROG_YACC
AC_PROG_AWK
AM_PROG_LEX
AC_PROG_CPP
AC_PROG_GCC_TRADITIONAL

case $host in
    alpha*-dec-osf*)
	CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-D_XOPEN_SOURCE_EXTENDED -DBYTE_ORDER=LITTLE_ENDIAN -D_POSIX_SOURCE -D_POSIX_C_SOURCE=199309L -D_OSF_SOURCE"
	;;

    *-*-hpux*)
	#HPUX needs _PROTOTYPES to include prototypes
	#for configure (for gcc and cc)
	CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-D_PROTOTYPES"
	;;

    *-*-solaris*)
	#for msghdr msg_flags
	CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-D_XOPEN_SOURCE=600"
	CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-D__EXTENSIONS__ -DBSD_COMP"
	;;

    *-*-linux-*)
	CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED"
	CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-D_BSD_SOURCE -D_DEFAULT_SOURCE"
	;;

    *-*-aix*)
	AC_DEFINE(_ALL_SOURCE, 1, [contents from old AC_AIX test])
	CPPFLAGS="${CPPFLAGS}${CPPFLAGS+ }-DXOPEN_SOURCE_EXTENDED=1"
	;;
esac

#XXX cross compilation
case $host_alias in
    arm-linux-androideabi)
	LDFLAGS="$LDFLAGS${LDFLAGS:+ }-lgcc -ldl"
	;;
esac

#XXX only needed for libraries
case $host in
    *-*-darwin*)
	if test x"${enable_dependency_tracking}" = xno; then
	    HW=`uname -m`
	    case $HW in
		ppc*)
		    CFLAGS="$CFLAGS${CFLAGS:+ }-arch ppc -arch ppc64"
		    LDFLAGS="$LDLAGS${LDLAGS:+ }-arch ppc -arch ppc64"
		    ;;
		*)
		    CFLAGS="$CFLAGS${CFLAGS:+ }-arch i386 -arch x86_64"
		    LDFLAGS="$LDLAGS${LDLAGS:+ }-arch i386 -arch x86_64"
		    ;;
	    esac
	fi
esac

unset COMPTYPE
unset FAILWARN
AC_MSG_CHECKING([for compiler type])
changequote(<<, >>)dnl
if $CC -v 2>&1 | tail -1 | grep -E '^gcc ' >/dev/null; then
    COMPTYPE=gcc
    FAILWARN="-Wall -Werror"
elif $CC -v 2>&1 | grep -E '(^|^[a-zA-Z]+ )(clang|LLVM) ' >/dev/null; then
    COMPTYPE=clang
    FAILWARN="-Wall -Werror"
elif $CC -V 2>&1 | grep 'Sun C ' >/dev/null; then
    COMPTYPE=suncc
    FAILWARN="-v -errwarn=%all"
elif $CC -v 2>&1 | grep -E '^pcc ' >/dev/null; then
    COMPTYPE=pcc
elif $CC -qversion 2>&1 | grep -E '^IBM XL C' >/dev/null; then
    COMPTYPE=xlc
    FAILWARN="-qhalt=w"
else
    #XXX
    case $host in
	alphaev6-dec-osf*)
	    COMPTYPE="osfcc"
	    ;;
	alpha*-dec-osf*)
	    COMPTYPE="oldosfcc"
	    ;;
	*-*-irix*) #sgi cc
	    COMPTYPE="sgicc"
	    ;;
	*-*-hpux*)
	    COMPTYPE="hpuxcc"
	    ;;
	*)
	    #gcc compatible compiler?
	    if test x"$GCC" != x; then
		COMPTYPE="gcc"
	    fi
	    ;;
    esac
fi
changequote([, ])dnl
if test x"$COMPTYPE" = x; then
    AC_MSG_RESULT([unknown])
else
    AC_MSG_RESULT([$COMPTYPE])
fi

AC_MSG_CHECKING([for preprocessor flags])
unset cpp_flags
case $ac_cv_prog_CPP in
    *gcc*)
	#simplify parsing after cpp processing (for e.g. errno checks)
	cpp_flags="${cpp_flags}${cpp_flags:+ }-P"

	if test x"`uname`" = xSunOS; then
	    cpp_flags="${cpp_flags}${cpp_flags:+ }-std=gnu99"
	fi
	;;
    suncc*)
	if test x"`uname`" = xSunOS; then
	    cpp_flags="${cpp_flags}${cpp_flags:+ }-xc99=all"
	fi
	;;
esac
if test x"$cpp_flags" != x; then
   CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }$cpp_flags"
   AC_MSG_RESULT([$cpp_flags])
else
   AC_MSG_RESULT([none])
fi

unset comp_flags
AC_MSG_CHECKING([for compiler flags])
case $COMPTYPE in
    gcc)
	if test x"`uname`" = xSunOS; then
	    comp_flags="-std=gnu99"
	fi
	;;

    suncc)
	#-xs provides easier debugging with gdb
	comp_flags="-Xa -xs"
	if test x"`uname`" = xSunOS; then
	    comp_flags="$comp_flags -xc99=all"
	fi
    ;;

    osfcc)
	comp_flags="-std1"
    ;;

    oldosfcc) #XXX is it possible to get it to work with -newc?
	if test x"$GCC" = x; then
	    comp_flags="-std1 -oldc"
	fi
    ;;

    hpuxcc)
	if test x"$GCC" = x; then
	    CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-D_XOPEN_SOURCE"
	    #XXX when cc is used as CPP it needs -Ae to work
	    #    for L_SOCKPROTO; add -Ae to CPPFLAGS.
	    #    This won't work if CPP is specified by hand
	    #    and is something else than cc (when CC is hp cc)
#	    comp_flags="-Ae"
	    CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-Ae"
	fi
	;;

    xlc)
	comp_flags=""
	;;
esac
#make sure compiling with compiler options works
if test x"$comp_flags" != x; then
    oCFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS${CFLAGS:+ }$comp_flags"
    AC_TRY_COMPILE([], [],
		   [AC_MSG_RESULT([$comp_flags])],
		   [AC_MSG_RESULT([none])
		    unset comp_flags])
    CFLAGS="$oCFLAGS"
else
    AC_MSG_RESULT([none])
fi

case $COMPTYPE in
    suncc)
	true #skip on this platform, gives warning but does not fail
	;;
    *)
	AC_MSG_CHECKING([for support for -pipe compiler flag])
	oCFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN -pipe"
	AC_TRY_RUN([
int main()
{
	return 0;
	}], [AC_MSG_RESULT([yes])
	     comp_flags="${comp_flags}${comp_flags:+ }-pipe"],
	    [AC_MSG_RESULT([no])],
	    [dnl do not set when cross-compiling
	     AC_MSG_RESULT([no])])
	CFLAGS="$oCFLAGS"
	;;
esac

AC_MSG_CHECKING([for support for -Wbounded compiler flag])
case $COMPTYPE in
    xlc)
	#use of -Wbounded does not result in an error even when
	#-qhalt=w is specified, resulting in a warning for each
	#compiled file, so disable check
	AC_MSG_RESULT([no, disabled for this compiler])
	;;
    *)
	oCFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN -Wbounded"
	AC_TRY_RUN([
int main()
{
        return 0;
}],     [AC_MSG_RESULT([yes])
	 comp_flags="${comp_flags}${comp_flags:+ }-Wbounded"
	 AC_DEFINE(HAVE_DECL_BOUNDED, 1, [__bounded__ macro support])],
	   [AC_MSG_RESULT([no])],
	   [AC_MSG_RESULT([no]) dnl assume not supported when cross-compiling])
	CFLAGS="$oCFLAGS"
	;;
esac

AC_MSG_CHECKING([whether compiler supports _Pragma()])
oCFLAGS="$CFLAGS"
CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN"
AC_TRY_COMPILE([
#include <stdlib.h>

#define foo(x)                                            \
do {                                                      \
 _Pragma("GCC diagnostic ignored \"-Waddress\"");         \
} while (x > 1)
], [foo(0)],
    [AC_MSG_RESULT([yes])
     AC_DEFINE(HAVE_PRAGMA_SUPPORT, 1, [_Pragma() supported by compiler])],
    [AC_MSG_RESULT([no])])
CFLAGS="$oCFLAGS"

AC_MSG_CHECKING([for __attribute__ support])
oCFLAGS="$CFLAGS"
CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN"
AC_TRY_RUN([
#include <stdlib.h>

void errfunc(void) __attribute__((noreturn));

void errfunc(void)
{
    exit(0);
}

int main()
{
    errfunc();
}], [AC_MSG_RESULT([yes])
     AC_DEFINE(HAVE_DECL_ATTRIBUTE, 1, [__attribute__ macro support])],
    [AC_MSG_RESULT([no])],
    [AC_MSG_RESULT([no]) dnl assume not supported when cross-compiling])
CFLAGS="$oCFLAGS"

AC_MSG_CHECKING([for constructor __attribute__ support])
oCFLAGS="$CFLAGS"
CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN"
AC_TRY_RUN([
#include <stdlib.h>

void errfunc(void) __attribute__((constructor));

void errfunc(void)
{
    exit(0);
}

int main()
{
    errfunc();
}], [AC_MSG_RESULT([yes])
     AC_DEFINE(HAVE_DECL_CONSTRUCTOR, 1, [constructor __attribute__ support])],
    [AC_MSG_RESULT([no])],
    [AC_MSG_RESULT([no]) dnl assume not supported when cross-compiling])
CFLAGS="$oCFLAGS"

AC_MSG_CHECKING([for __attribute__ nonnull support])
oCFLAGS="$CFLAGS"
CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN"
AC_TRY_RUN([
#include <stdlib.h>

void func(char *) __attribute__((__nonnull__(1)));

void func(char *f)
{
    char *d;
    d = f;
}

int main()
{
    func(NULL);
}], [AC_MSG_RESULT([yes])
     AC_DEFINE(HAVE_DECL_NONNULL, 1, [__nonnull__ attribute support])],
    [AC_MSG_RESULT([no])],
    [AC_MSG_RESULT([no]) dnl assume not supported when cross-compiling])
CFLAGS="$oCFLAGS"

AC_MSG_CHECKING([for __printf__ format attribute support])
oCFLAGS="$CFLAGS"
CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN"
AC_TRY_RUN([
#include <stdlib.h>

void func(const char *fmt, ...)
   __attribute__((format(__printf__, 1, 2)));

void func(const char *fmt, ...) {
     (void)fmt;
     return;
}

int main()
{
    func("foo");
    return 0;
}], [AC_MSG_RESULT([yes])
     AC_DEFINE(HAVE_DECL_FORMAT, 1, [format attribute support])],
    [AC_MSG_RESULT([no])],
    [AC_MSG_RESULT([no]) dnl assume not supported when cross-compiling])
CFLAGS="$oCFLAGS"

#check whether to enable debugging
unset NODEBUG
unset debug_enabled
AC_MSG_CHECKING([for compilation with debugging])
AC_ARG_ENABLE(debug,
[  --enable-debug          compile with debugging support],
[if test x"$enableval" = xno; then
    NODEBUG="Disabled, using --disable-debug"
 fi],
[#off by default for full release, enabled by default for prerelease
 if test x"$prerelease" = x; then
    NODEBUG="Disabled (default)"
 fi])
if test x"$NODEBUG" != x; then
    AC_MSG_RESULT([disabled])
else
    AC_MSG_RESULT([yes])
    debug_enabled=t
    FEAT="$FEAT${FEAT:+ }debug"
fi

#check whether to enable livedebug
unset NOLIVEDEBUG
AC_MSG_CHECKING([for live debugging])
AC_ARG_ENABLE(livedebug,
[  --enable-livedebug      enable low-overhead debugging mode],
[if test x"$enableval" = xno; then
    NOLIVEDEBUG="Disabled, using --disable-livedebug"
 fi],
[#off by default for full release, enabled by default for prerelease
 if test x"$prerelease" = x; then
    NOLIVEDEBUG="Disabled (default)"
 fi])
if test x"$NOLIVEDEBUG" != x; then
    AC_MSG_RESULT([disabled])
else
    AC_MSG_RESULT([yes])
    debug_enabled=t
    FEAT="$FEAT${FEAT:+ }livedebug"
    AC_DEFINE(HAVE_COND_LIVEDEBUG, 1, [low-overhead debugging enabled])
fi

#check for problem with linker/gcc on AIX
unset aixldbug
case $COMPTYPE in
    gcc)
	oCFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS${CFLAGS:+ }-g"
	AC_MSG_CHECKING([whether compiling using -g works with gcc])
	AC_TRY_LINK([int foo;], [foo++;],
	    [AC_MSG_RESULT(yes)],
	    [AC_MSG_RESULT(no)
             AC_MSG_WARN([building with -g0])
	     aixldbug=t])
        CFLAGS="$oCFLAGS"
        ;;
esac

#enable -fstack-protector if debug enabled
#XXX disabled, can lead to build failure on some platforms
#if test x"${debug_enabled}" != x; then
#   AC_MSG_CHECKING([for support for -fstack-protector compiler flag])
#   oCFLAGS="$CFLAGS"
#   CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN -fstack-protector"
#   AC_TRY_RUN([
#int main()
#{
#        return 0;
#}], [AC_MSG_RESULT([yes])
#     comp_flags="${comp_flags}${comp_flags:+ }-fstack-protector"],
#    [AC_MSG_RESULT([no])],
#    [AC_MSG_RESULT([no]) dnl assume not supported when cross-compiling])
#   CFLAGS="$oCFLAGS"
#fi

#set compilation debugging flags
if test x"${debug_enabled}" = xt; then
    #no optimization wanted
    if test x"${ac_cv_prog_cc_g}" = xyes; then
	case $COMPTYPE in
	    gcc)
		if test x"$aixldbug" != x; then
		    #disable debug info
		    CFLAGS="$CFLAGS${CFLAGS:+ }-g0"
		else
		    CFLAGS="$CFLAGS${CFLAGS:+ }-ggdb"
		fi
		;;
	    *)
		CFLAGS="$CFLAGS${CFLAGS:+ }-g"
		;;
	esac
    fi
    CPPFLAGS="$CPPFLAGS${CPPFLAGS:+ }-DDEBUG=1"
else
    #autoconf_compflags is set to "-g -O2" with GCC
    #override CFLAGS when running configure to avoid this
    CPPFLAGS="$CPPFLAGS${CPPFLAGS:+ }-DDEBUG=0"
    CFLAGS="$CFLAGS${CFLAGS:+ }$autoconf_compflags"

    case $COMPTYPE in
	gcc)
	    if test x"$aixldbug" != x; then
	        #disable debug info
		if echo $CFLAGS | grep -- "-g " >/dev/null; then
		    CFLAGS="`echo $CFLAGS | sed -e 's/-g //g'`"
		fi
		CFLAGS="$CFLAGS${CFLAGS:+ }-g0"
	    else
		#use -ggdb also when not debugging
		if echo $CFLAGS | grep -- "-g " >/dev/null; then
		    CFLAGS="`echo $CFLAGS | sed -e 's/-g //g'`"
		fi
		CFLAGS="$CFLAGS${CFLAGS:+ }-ggdb"
	    fi
	    ;;
    esac
fi

#check whether to compilation warnings
unset NOWARN
AC_MSG_CHECKING([for warning flags])
AC_ARG_ENABLE(warnings,
[  --enable-warnings       show compilation warnings],
[if test x"$enableval" = xno; then
    NOWARN="Disabled, using --disable-warnings"
 fi],
[#off by default
 NOWARN="Disabled (default)"])

#check whether to enable extra compilation warnings
unset NOWARN
AC_ARG_ENABLE(warnings,
[  --enable-warnings       show compilation warnings],
[if test x"$enableval" = xno; then
    NOWARN="Disabled, using --disable-warnings"
 fi],
[#off by default
 NOWARN="Disabled (default)"])

#place warning flags in $warn
unset warn
if test x"$NOWARN" = x; then
    #try to enable compiler specific warning flags
    case $COMPTYPE in
	gcc | clang)
	    warn="-Wall -Wformat=2 -Wformat-truncation=2 -Wformat-overflow=2 -Wnested-externs -Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes -Wcast-align -Wcast-qual -Wbad-function-cast -Wpointer-arith -Wundef -Wunused-but-set-variable -Wnull-dereference  -Wmisleading-indentation -Wmaybe-uninitialized -Wstring-compare -Warray-bounds=2 -Wduplicated-branches -Wfloat-equal -Wshadow -Wextra -Wbad-function-cast -Wcast-align -Wwrite-strings -Wconversion -Wno-sign-conversion -Wlogical-op -fno-eliminate-unused-debug-symbols -fanalyzer"
	    ;;

	suncc)
	    warn="-v"
	    case $host in
		#XXX only available for some platforms
		sparc-*solaris*)
		    warn="$warn -xanalyze=code"
		    ;;
	    esac
	    ;;

	xlc)
	    warn="-qinfo=all:noppt"
	    ;;

	*) #try -Wall (gcc)
	    warn="-Wall"
	    ;;
    esac
else
    #default case, only subset of warnings
    case $COMPTYPE in
	gcc | clang)
	    warn="-Wformat=2"
	    ;;
    esac
fi

#make sure compilation is still possible
unset okwarn
for flag in $warn; do
    oCFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS${CFLAGS:+ }$flag $FAILWARN"
    AC_MSG_CHECKING([for warning flags $flag])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[AC_MSG_RESULT([yes])
		    okwarn="$okwarn${okwarn:+ }$flag"],
         	    [AC_MSG_RESULT([no])])
	CFLAGS="$oCFLAGS"
done
warn="$okwarn"

#check if compilation with FORTIFY_SOURCE gives error/warning
AC_MSG_CHECKING([whether compilation with FORTIFY_SOURCE works])
oCFLAGS="$CFLAGS"
oCPPFLAGS="$CPPFLAGS"
CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN"
CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-D_FORTIFY_SOURCE=2"
AC_TRY_COMPILE([#include <stdio.h>], [],
		   [AC_MSG_RESULT([yes])],
		   [AC_MSG_RESULT([no])
		    CPPFLAGS="$oCPPFLAGS"])
CFLAGS="$oCFLAGS"

#for Solaris, generate 64-bit binaries if running in 64-bit mode.
#building 32-bit binaries (the default) causes problems with
#LD_PRELOAD if running in a 64-bit environment.
#XXX more general solution would be to build and install both
unset sol64
case $host in
    *-*-solaris*)
	if test x`isainfo -b` = x64; then
	    ISA64DIR=`isainfo -n`
	    if test x"$ISA64DIR" != x; then
		AC_MSG_CHECKING([for support for -m64 compiler flag])
		oCFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS${CFLAGS:+ }-m64"
		AC_TRY_RUN([
			int main() {
			    return 0;
		}], [AC_MSG_RESULT([yes])
		     sol64=t
		     AC_SUBST(ISA64DIR)
		     AC_MSG_WARN([building for 64-bit addressing model])])
		CFLAGS="$oCFLAGS"
	    fi
	fi
	;;
esac

#NOTE: set warnings at the bottom; might interfere with tests
CFLAGS="$CFLAGS${CFLAGS:+ }$comp_flags"

#check whether to compile with extra DIAGNOSTICS
unset NODIAG
AC_MSG_CHECKING([for compliation with DIAGNOSTIC])
AH_TEMPLATE([DIAGNOSTIC], [for debugging])
AC_ARG_ENABLE(diagnostic,
[  --enable-diagnostic     enable diagnostic],
[if test x"$enableval" = xno; then
    NODIAG="Disabled, using --disable-diagnostics"
 fi],
[#off by default for full release, enabled by default for prerelease
 if test x"$prerelease" = x; then
    NODIAG="Disabled (default)"
 fi])
if test x"$NODIAG" = x; then
    FEAT="$FEAT${FEAT:+ }diagnostic"
    AC_DEFINE(DIAGNOSTIC, 1)
    AC_MSG_RESULT([yes])
else
    AC_DEFINE(DIAGNOSTIC, 0)
    AC_MSG_RESULT([no])
fi

#check whether to build with profiling
unset NOPROFIL
AC_CHECK_FUNCS(moncontrol)
AC_MSG_CHECKING([whether profiled compilation requested])
AC_ARG_ENABLE(profiling,
[  --enable-profiling      compile with profiling support in server],
[if test x"$enableval" = xno; then
    NOPROFIL="Disabled, using --disable-profiling"
 fi],
[#off by default
 NOPROFIL="Disabled (default)"])
if test x"$NOPROFIL" = x; then
    AC_MSG_RESULT([yes])

    oLDFLAGS="$LDFLAGS"
    oCFLAGS="$CFLAGS"
    LDFLAGS="$LDFLAGS${LDFLAGS:+ }-pg"
    CFLAGS="$CFLAGS${CFLAGS:+ }-pg -DPROFILING"
    AC_TRY_RUN([
int main()
{
	return 0;
}], [FEAT="$FEAT${FEAT:+ }profiling"
     AC_DEFINE(HAVE_PROFILING, 1, [for profiling])

     case $host in
	 *-*-openbsd* | *-*-freebsd*)
	     #static linking, disable server preloading
	     no_preload_server=t
	     #OpenBSD and FreeBSD appear to be happier if -lc is included
	     #when profiling is enabled
	     LIBS="$LIBS${LIBS:+ }-lc"
	     ;;
     esac],
   [AC_MSG_WARN([profiling requested, but compilation with profiling fails])
    NOPROFIL="Disabled, profiled compilation fails"
    CFLAGS="$oCFLAGS"
    LDFLAGS="$oLDFLAGS"])
else
    AC_MSG_RESULT([no])
fi

#check whether to build with coverage
unset NOCOVERAGE
AC_MSG_CHECKING([whether coverage requested])
AC_ARG_ENABLE(coverage,
[  --enable-coverage       compile with coverage],
[if test x"$enableval" = xno; then
    NOCOVER="Disabled, using --disable-coverage"
 fi],
[#disabled by default
 NOCOVER="Disabled (default)"])
if test x"$NOCOVER" = x; then
    AC_MSG_RESULT([yes])
    oLDFLAGS="$LDFLAGS"
    oCFLAGS="$CFLAGS"
    LDFLAGS="$LDFLAGS${LDFLAGS:+ }--coverage"
    CFLAGS="$CFLAGS${CFLAGS:+ }--coverage"
    AC_TRY_RUN([
#include <sys/types.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <unistd.h>

int main()
{
	int res;
	/* look for darwin() fork problem */
	if ((res = fork()) == 0)
	    exit(0); /* child */
	else if (res == -1)
	    exit(1); /* err */
	else {
	    if (waitpid(res, NULL, 0) == res)
		exit(0);
	    else
		exit(1);
	}
}], [FEAT="$FEAT${FEAT:+ }coverage"
     AC_DEFINE(HAVE_COVERAGE, 1, [for code coverage])],
    [AC_MSG_WARN([coverage requested, but compilation with --coverage fails])
     NOCOVER="Disabled, --coverage compilation fails"
     CFLAGS="$oCFLAGS"
     LDFLAGS="$oLDFLAGS"])
else
    AC_MSG_RESULT([no])
fi

#check whether to run lint
unset NOLINT
AC_MSG_CHECKING([whether linting requested])
AC_ARG_ENABLE(linting,
[  --enable-linting        enable lint],
[if test x"$enableval" = xno; then
    NOLINT="Disabled, using --disable-linting"
 fi],
[#off by default
 NOLINT="Disabled (default)"])
if test x"$NOLINT" = x; then
    AC_MSG_RESULT([yes])

    AC_CHECK_PROG(LINT, lint, lint)
    if test x"$LINT" = x; then
	AC_MSG_WARN([linting requested, but lint not found])
	NOLINT="Disabled, lint not found"
    else
	s_linting=t
	case $host in
	    *-*-aix*)
		LINTFLAGS="-abcbpx -Nn8000 -Nd8000"
		LINTPASS1=""
		LINTPASS2=""
		LINTLIBOPT="-C"
		;;
	    *-*-openbsd* | *-*-freebsd*)
		LINTFLAGS="-abcebprxz"
		LINTPASS1="-i"
		LINTPASS2=""
		LINTLIBOPT="-C"
		;;
	    *-*-solaris*)
#		LINTFLAGS=-c -errchk=%all -errsecurity=extended -fd -Ncheck=%all -Nlevel=3 -p -s
#		SUPPRESS="-x -erroff=E_FUNC_DECL_VAR_ARG2"
		LINTLIBS="-lnsl -lsocket -lwrap"
		LINTWARN="-errsecurity=extended -errchk=%all -errhdr -Ncheck=%all -Nlevel=3"
		LINTFLAGS="-fd -s -errfmt=simple $SUPPRESS $LINTWARN"
		LINTPASS1="-c"
		LINTPASS2="$LINTFLAGS $LINTLIBS"
		LINTLIBOPT="-o"
		;;
	esac
    fi
else
    AC_MSG_RESULT([no])
fi
AM_CONDITIONAL(RUNLINT, test x"$NOLINT" = x)
AC_SUBST(LINT)
AC_SUBST(LINTFLAGS)
AC_SUBST(LINTPASS1)
AC_SUBST(LINTPASS2)
AC_SUBST(LINTLIBOPT)
